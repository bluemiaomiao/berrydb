/*******************************************************************************
                  Copyright (C) 2021 BerryDB Software Inc.
This application is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License, Version 3, as
published by the Free Software Foundation.

This application is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR PARTICULAR PURPOSE, See the GNU Affero General Public License for more
details.

You should have received a copy of the GNU Affero General Public License along
with this application. If not, see <http://www.gnu.org/license/>
*******************************************************************************/

#include "data_management.hpp"
#include "problem_detect.hpp"

using namespace bson;

const char *gKeyFieldName = DATA_MANAGEMENT_KEY_FIELDDNAME;

DataManagementFile::DataManagementFile(IndexManagementBucketManager *ixmBucketMgr) : _header(NULL), _pFileName(NULL) {
    this->_ixmBucketMgr = ixmBucketMgr;
}

DataManagementFile::~DataManagementFile() {
    if (this->_pFileName) {
        free(this->_pFileName);
    }
    close();
}

int DataManagementFile::insert(BSONObj &record, BSONObj &outRecord, DataManagementRecordID &rid) {
    int rc = DB_OK;
    PAGEID pageID = 0;
    char *page = NULL;
    DataManagementPageHeader *pageHeader = NULL;
    int recordSize = 0;
    SLOTOFF offsetTemp = 0;
    const char *pGKeyFieldName = NULL;
    DataManagementRecord recordHeader;

    recordSize = record.objsize();

    if ((unsigned int) recordSize > DATA_MANAGEMENT_MAX_RECORD) {
        rc = DB_INVALID_ARG;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "record cannot bigger than 4MB");
        goto error;
    }

    pGKeyFieldName = gKeyFieldName;

    if (record.getFieldDottedOrArray(pGKeyFieldName).eoo()) {
        rc = DB_INVALID_ARG;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "record must be with _id");
        goto error;
    }

    retry:
        this->_mutex.get();
        pageID = this->_findPage(recordSize + sizeof(DataManagementRecord));

        if (DATA_MANAGEMENT_INVALID_PAGEID == pageID) {
            this->_mutex.release();

            if (this->_extendMutex.try_get()) {
                rc = this->_extendSegment();
                if (rc) {
                    PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to extend segment, rc = %d", rc);
                    this->_extendMutex.release();
                    goto error;
                }
            } else {
                this->_extendMutex.get();
            }

            this->_extendMutex.release();
            goto retry;
        }
        page = this->pageToOffset(pageID);

        if (!page) {
            rc = DB_SYS;
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to find the page");
            goto error_releasemutex;
        }

        pageHeader = (DataManagementPageHeader *) page;

        if (memcmp(pageHeader->_eyeCatcher, DATA_MANAGEMENT_PAGE_EYECATCHER, DATA_MANAGEMENT_PAGE_EYECATCHER_LEN) != 0) {
            rc = DB_SYS;
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid page header");
            goto error_releasemutex;
        }

        if ((pageHeader->_freeSpace > pageHeader->_freeOffset - pageHeader->_slotOffset) &&
        (pageHeader->_slotOffset + recordSize + sizeof(DataManagementRecord) + sizeof(SLOTID) > pageHeader->_freeOffset)) {
            this->_recoverSpace(page);
        }

        if ((pageHeader->_freeSpace < recordSize + sizeof(DataManagementRecord) + sizeof(SLOTID)) ||
        (pageHeader->_freeOffset - pageHeader->_slotOffset < recordSize + sizeof(DataManagementRecord) + sizeof(SLOTID))) {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Something big wrong!!");
            rc = DB_SYS;
            goto error_releasemutex;
        }

        offsetTemp = pageHeader->_freeOffset - recordSize - sizeof(DataManagementRecord);

        recordHeader._size = recordSize + sizeof(DataManagementRecord);
        recordHeader._flag = DATA_MANAGEMENT_FLAG_NORMAL;

        *(SLOTOFF *) (page + sizeof(DataManagementPageHeader) + pageHeader->_numSlots * sizeof(SLOTOFF)) = offsetTemp;

        memcpy(page + offsetTemp, (char *) &recordHeader, sizeof(DataManagementRecord));

        memcpy(page + offsetTemp + sizeof(DataManagementRecord), record.objdata(), recordSize);
        outRecord = BSONObj(page + offsetTemp + sizeof(DataManagementRecord));

        rid._pageID = pageID;
        rid._slotID = pageHeader->_numSlots;

        pageHeader->_numSlots++;
        pageHeader->_slotOffset += sizeof(SLOTID);
        pageHeader->_freeOffset = offsetTemp;

        this->_updateFreeSpace(pageHeader,-(recordSize + sizeof(SLOTID) + sizeof(DataManagementRecord)), pageID);

        this->_mutex.release();
    done:
        return rc;
    error_releasemutex:
        this->_mutex.release();
    error:
        goto done;
}

int DataManagementFile::remove(DataManagementRecordID &rid) {
    int rc = DB_OK;
    SLOTOFF slot = 0;
    char *page = NULL;
    DataManagementRecord *recordHeader = NULL;
    DataManagementPageHeader *pageHeader = NULL;
    std::pair<std::multimap<unsigned int, PAGEID>::iterator, std::multimap<unsigned int, PAGEID>::iterator> ret;

    this->_mutex.get();

    page = this->pageToOffset(rid._pageID);

    if (!page) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to find the apge for %u;%u", rid._pageID, rid._slotID);
        goto error;
    }

    rc = this->_searchSlot(page, rid, slot);
    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to search slot, rc = %d", rc);
        goto error;
    }

    if (DATA_MANAGEMENT_SLOT_EMPTY == slot) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "The record is dropped");
        goto error;
    }

    pageHeader = (DataManagementPageHeader *) page;

    *(SLOTID *) (page + sizeof(DataManagementPageHeader) + rid._slotID * sizeof(SLOTID)) = DATA_MANAGEMENT_SLOT_EMPTY;

    recordHeader = (DataManagementRecord *) (page + slot);
    recordHeader->_flag = DATA_MANAGEMENT_FLAG_DROPPED;

    this->_updateFreeSpace(pageHeader, recordHeader->_size, rid._pageID);

    done:
        this->_mutex.release();
        return rc;
    error:
        goto done;
}

int DataManagementFile::find(DataManagementRecordID &rid, BSONObj &result) {
    int rc = DB_OK;
    SLOTOFF slot = 0;
    char *page = NULL;
    DataManagementRecord *recordHeader = NULL;

    this->_mutex.get_shared();

    page = pageToOffset(rid._pageID);
    if (!page) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to find the page");
        goto error;
    }

    rc = _searchSlot(page, rid, slot);
    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to search slot, rc = %d", rc);
        goto error;
    }

    if (DATA_MANAGEMENT_SLOT_EMPTY == slot) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "The record is dropped");
        goto error;
    }

    recordHeader = (DataManagementRecord *) (page + slot);

    if (DATA_MANAGEMENT_FLAG_DROPPED == recordHeader->_flag) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "This data is dropped");
        goto error;
    }

    result = BSONObj(page + slot + sizeof(DataManagementRecord)).copy();

    done:
        this->_mutex.release_shared();
        return rc;
    error:
        goto done;
}

void DataManagementFile::_updateFreeSpace(DataManagementPageHeader *header, int changeSize, PAGEID pageID) {
    unsigned int freeSpace = header->_freeSpace;
    std::pair<std::multimap<unsigned int, PAGEID>::iterator, std::multimap<unsigned int, PAGEID>::iterator> ret;
    ret = this->_freeSpaceMap.equal_range(freeSpace);

    for (std::multimap < unsigned int, PAGEID>::iterator it = ret.first; it != ret.second; ++it) {
        if (it->second == pageID) {
            this->_freeSpaceMap.erase(it);
            break;
        }
    }

    freeSpace += changeSize;
    header->_freeSpace = freeSpace;

    this->_freeSpaceMap.insert(pair<unsigned int, PAGEID>(freeSpace, pageID));
}

int DataManagementFile::initialize(const char *pFileName) {
    offsetType offset = 0;
    int rc = DB_OK;
    this->_pFileName = strdup(pFileName);
    if (!this->_pFileName) {
        rc = DB_OOM;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to duplicate file name");
        goto error;
    }

    rc = open(this->_pFileName, OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_ALWAYS);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to open file %s, rc = %d", this->_pFileName, rc);

    getfilesize:
        rc = this->_fileOp.getSize(&offset);
        PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to get file size, rc = %d", rc);

        if (!offset) {
            rc = this->_initNew();
            PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to initialize file, rc = %d", rc);
            goto getfilesize;
        }

        rc = this->_loadData();
        PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to load data, rc = %d", rc);
    done:
        return rc;
    error:
        goto done;
}

int DataManagementFile::_extendSegment() {

    int rc = DB_OK;
    char *data = NULL;
    int freeMapSize = 0;
    DataManagementPageHeader pageHeader;
    offsetType offset = 0;

    rc = _fileOp.getSize(&offset);

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to get file size, rc = %d", rc);

    rc = _extendFile(DATA_MANAGEMENT_FILE_SEGMENT_SIZE);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to extend segment rc = %d", rc);

    rc = map(offset, DATA_MANAGEMENT_FILE_SEGMENT_SIZE, (void **) &data);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to map file, rc = %d", rc);

    strcpy(pageHeader._eyeCatcher, DATA_MANAGEMENT_PAGE_EYECATCHER);

    pageHeader._size = DATA_MANAGEMENT_PAGESIZE;
    pageHeader._flag = DATA_MANAGEMENT_FLAG_NORMAL;
    pageHeader._numSlots = 0;
    pageHeader._slotOffset = sizeof(DataManagementPageHeader);
    pageHeader._freeSpace = DATA_MANAGEMENT_PAGESIZE - sizeof(DataManagementPageHeader);
    pageHeader._freeOffset = DATA_MANAGEMENT_PAGESIZE;

    for (int i = 0; i < DATA_MANAGEMENT_FILE_SEGMENT_SIZE; i += DATA_MANAGEMENT_PAGESIZE) {
        memcpy(data + i, (char *) &pageHeader, sizeof(DataManagementPageHeader));
    }

    this->_mutex.get();

    freeMapSize = this->_freeSpaceMap.size();

    for (int i = 0; i < DATA_MANAGEMENT_PAGES_PER_SEGMENT; ++i) {
        this->_freeSpaceMap.insert(pair<unsigned int, PAGEID> (pageHeader._freeSpace,i + freeMapSize));
    }

    this->_body.push_back(data);
    this->_header->_size += DATA_MANAGEMENT_PAGES_PER_SEGMENT;
    this->_mutex.release();

    done :
        return rc;
    error :
        goto done;
}

int DataManagementFile::_initNew() {
    int rc = DB_OK;
    rc = _extendFile(DATA_MANAGEMENT_FILE_HEADER_SIZE);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to extend file, rc = %d", rc);
    rc = map(0, DATA_MANAGEMENT_FILE_HEADER_SIZE, (void **) &this->_header);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to map, rc = %d", rc);

    strcpy(this->_header->_eyeCatcher, DATA_MANAGEMENT_HEADER_EYECATCHER);
    this->_header->_size = 0;
    this->_header->_flag = DATA_MANAGEMENT_HEADER_FLAG_NORMAL;
    this->_header->_version = DATA_MANAGEMENT_HEADER_VERSION_CURRENT;

    done:
        return rc;
    error:
        goto done;
}

PAGEID DataManagementFile::_findPage(size_t requiredSize) {
    std::multimap<unsigned int, PAGEID>::iterator findIter;
    findIter = this->_freeSpaceMap.upper_bound(requiredSize);

    if (findIter != this->_freeSpaceMap.end()) {
        return findIter->second;
    }

    return DATA_MANAGEMENT_INVALID_PAGEID;
}

int DataManagementFile::_extendFile(int size) {
    int rc = DB_OK;
    char temp[DATA_MANAGEMENT_EXTEND_SIZE] = {0};

    memset(temp, 0, DATA_MANAGEMENT_EXTEND_SIZE);

    if (size % DATA_MANAGEMENT_EXTEND_SIZE != 0) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid extend size, must be multiple of %d",
                            DATA_MANAGEMENT_EXTEND_SIZE);
        goto error;
    }

    for (int i = 0; i < size; i += DATA_MANAGEMENT_EXTEND_SIZE) {
        this->_fileOp.seekToEnd();
        rc = this->_fileOp.Write(temp, DATA_MANAGEMENT_EXTEND_SIZE);
        PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to write to file, rc = %d", rc);
    }

    done:
        return rc;
    error:
        goto done;
}

int DataManagementFile::_searchSlot(char *page, DataManagementRecordID &rid, SLOTOFF &slot) {
    int rc = DB_OK;
    DataManagementPageHeader *pageHeader = NULL;

    if (!page) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "page is NULL");
        goto error;
    }

    if (0 > rid._pageID || 0 > rid._slotID) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid RID: %d.%d", rid._pageID, rid._slotID);
        goto error;
    }

    pageHeader = (DataManagementPageHeader *) page;

    if (rid._slotID > pageHeader->_numSlots) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Slot is out of range, provided: %d, max: %d",
                            rid._slotID, pageHeader->_numSlots);
        goto error;
    }
    slot = *(SLOTOFF *) (page + sizeof(DataManagementPageHeader) + rid._slotID * sizeof(SLOTOFF));

    done:
        return rc;
    error:
        goto done;
}

int DataManagementFile::_loadData() {
    int rc = DB_OK;
    int numPage = 0;
    int numSegments = 0;
    DataManagementPageHeader *pageHeader = NULL;
    char *data = NULL;
    BSONObj bson;
    SLOTID slotID = 0;
    SLOTOFF slotOffset = 0;
    DataManagementRecordID recordID;

    if (!this->_header) {
        rc = map(0, DATA_MANAGEMENT_FILE_HEADER_SIZE, (void **) &this->_header);
        PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to map file header, rc = %d", rc);
    }

    numPage = this->_header->_size;
    if (numPage % DATA_MANAGEMENT_PAGES_PER_SEGMENT) {
        rc = DB_SYS;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to load data, partial segments detected");
        goto error;
    }

    numSegments = numPage / DATA_MANAGEMENT_PAGES_PER_SEGMENT;

    if (numSegments > 0) {
        for (int i = 0; i < numSegments; ++i) {
            rc = map(DATA_MANAGEMENT_FILE_HEADER_SIZE + DATA_MANAGEMENT_FILE_SEGMENT_SIZE * i,
                     DATA_MANAGEMENT_FILE_SEGMENT_SIZE,(void **) &data);

            PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to map segment %d, rc = %d", i, rc);

            this->_body.push_back(data);

            for (unsigned int k = 0; k < DATA_MANAGEMENT_PAGES_PER_SEGMENT; ++k) {
                pageHeader = (DataManagementPageHeader *) (data + k * DATA_MANAGEMENT_PAGESIZE);

                this->_freeSpaceMap.insert(pair<unsigned int, PAGEID>(pageHeader->_freeSpace, k));

                slotID = (SLOTID) pageHeader->_numSlots;

                recordID._pageID = (PAGEID) k;

                for (unsigned int s = 0; s < slotID; ++s) {
                    slotOffset = *(SLOTOFF *) (data + k * DATA_MANAGEMENT_PAGESIZE +
                            sizeof(DataManagementPageHeader) + s * sizeof(SLOTID));

                    if (DATA_MANAGEMENT_SLOT_EMPTY == slotOffset) {
                        continue;
                    }

                    bson = BSONObj(data + k * DATA_MANAGEMENT_PAGESIZE +
                            slotOffset + sizeof(DataManagementRecord));

                    recordID._slotID = (SLOTID) s;

                    rc = this->_ixmBucketMgr->isIDExist(bson);
                    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to call isIDExist, rc = %d", rc);

                    rc = this->_ixmBucketMgr->createIndex(bson, recordID);
                    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to call ixm createIndex, rc = %d", rc);
                }
            }
        }
    }

    done:
        return rc;
    error:
        goto done;
}

void DataManagementFile::_recoverSpace(char *page) {
    char *pLeft = NULL;
    char *pRight = NULL;
    SLOTOFF slot = 0;
    int recordSize = 0;
    bool isRecover = false;
    DataManagementRecord *recordHeader = NULL;
    DataManagementPageHeader *pageHeader = NULL;

    pLeft = page + sizeof(DataManagementPageHeader);
    pRight = page + DATA_MANAGEMENT_PAGESIZE;

    pageHeader = (DataManagementPageHeader *) page;

    for (unsigned int i = 0; i < pageHeader->_numSlots; ++i) {
        slot = *((SLOTOFF *) (pLeft + sizeof(SLOTOFF) * i));

        if (DATA_MANAGEMENT_SLOT_EMPTY != slot) {
            recordHeader = (DataManagementRecord *) (page + slot);
            recordSize = recordHeader->_size;
            pRight -= recordSize;

            if (isRecover) {
                memmove(pRight, page + slot, recordSize);
                *((SLOTOFF *) (pLeft + sizeof(SLOTOFF) * i)) = (SLOTOFF) (pRight - page);
            }
        } else {
            isRecover = true;
        }
    }

    pageHeader->_freeOffset = pRight - page;
}

//inline unsigned int DataManagementFile::getNumSegments() {
//    return this->_body.size();
//}

//inline unsigned int DataManagementFile::getNumPages() {
//    return this->getNumSegments() * DATA_MANAGEMENT_PAGES_PER_SEGMENT;
//}

//inline char *DataManagementFile::pageToOffset(PAGEID pageID) {
//    if(pageID >= this->getNumPages()) {
//        return NULL;
//    }
//
//    return this->_body[pageID / DATA_MANAGEMENT_PAGES_PER_SEGMENT] + DATA_MANAGEMENT_PAGESIZE * (pageID & DATA_MANAGEMENT_PAGES_PER_SEGMENT);
//}

//inline bool DataManagementFile::validSize(size_t size) {
//    if(size < DATA_MANAGEMENT_FILE_HEADER_SIZE) {
//        return false;
//    }
//
//    size -= DATA_MANAGEMENT_FILE_SEGMENT_SIZE;
//
//    return (size % DATA_MANAGEMENT_FILE_SEGMENT_SIZE) == 0;
//}
