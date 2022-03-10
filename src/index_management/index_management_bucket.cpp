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

#include "core.hpp"
#include "problem_detect.hpp"
#include "os_service_hash.hpp"
#include "index_management_bucket.hpp"

int IndexManagementBucketManager::isIDExist(BSONObj &record) {
    int rc = DB_OK;
    unsigned int hashNum = 0;
    unsigned int random = 0;
    IndexManagementElementHash eleHash ;
    DataManagementRecordID recordID ;

    rc = this->_processData(record, recordID, hashNum, eleHash, random);
    if (rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "Failed to process data, rc = %d", rc);
        goto error ;
    }

    rc = this->_bucket[random]->isIDExist(hashNum, eleHash);
    if(rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "Failed to create index, rc = %d", rc);
        goto error ;
    }

    done:
        return rc;
    error:
        goto done;
}

int IndexManagementBucketManager::createIndex(BSONObj &record, DataManagementRecordID &recordID)
{
    int rc = DB_OK;
    unsigned int hashNum = 0;
    unsigned int random = 0;

    IndexManagementElementHash eleHash;

    rc = this->_processData(record, recordID, hashNum, eleHash, random) ;
    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to process data, rc = %d", rc);

    rc = this->_bucket[random]->createIndex(hashNum, eleHash);
    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to create index, rc = %d", rc);

    recordID = eleHash.recordId ;

    done:
        return rc;
    error:
        goto done;
}


int IndexManagementBucketManager::findIndex(BSONObj &record, DataManagementRecordID &recordID)
{
    int rc = DB_OK;
    unsigned int hashNum = 0;
    unsigned int random = 0;

    IndexManagementElementHash eleHash;

    rc = this->_processData(record, recordID, hashNum, eleHash, random);
    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to process data, rc = %d", rc);

    rc = this->_bucket[random]->findIndex(hashNum, eleHash) ;
    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to find index, rc = %d", rc);

    recordID = eleHash.recordId;

    done:
        return rc;
    error:
        goto done;
}

int IndexManagementBucketManager::removeIndex(BSONObj &record, DataManagementRecordID &recordID)
{
    int rc = DB_OK;
    unsigned int hashNum  = 0;
    unsigned int random = 0;

    IndexManagementElementHash eleHash;

    rc = _processData(record, recordID, hashNum, eleHash, random);
    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to process data, rc = %d", rc);

    rc = _bucket[random]->removeIndex(hashNum, eleHash);
    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to remove index, rc = %d", rc);

    recordID._pageID = eleHash.recordId._pageID;
    recordID._slotID = eleHash.recordId._slotID;

    done:
        return rc;
    error:
        goto done;
}

int IndexManagementBucketManager::_processData(BSONObj &record, DataManagementRecordID &recordID, unsigned int &hashNum,
                                               IndexManagementElementHash &eleHash, unsigned int &random)
{
    int rc = DB_OK ;
    BSONElement element  = record.getField(INDEX_MANAGEMENT_KEY_FIELDNAME);

    if(element.eoo() || (element.type() != NumberInt && element.type() != String)) {
        rc = DB_INVALID_ARG ;
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "record must be with _id");
        goto error ;
    }

    hashNum = osServiceHash(element.value(), element.valuesize());
    random = hashNum % INDEX_MANAGEMENT_HASH_MAP_SIZE;

    eleHash.data = element.rawdata();
    eleHash.recordId = recordID;

    done:
        return rc;
    error:
        goto done;
}

int IndexManagementBucketManager::IndexManagementBucket::isIDExist(unsigned int hashNum,
                                                                   IndexManagementElementHash &eleHash) {
    int rc = DB_OK;
    BSONElement destEle;
    BSONElement sourEle;

    IndexManagementElementHash existEle;

    std::pair<std::multimap<unsigned int, IndexManagementElementHash>::iterator, std::multimap<unsigned int, IndexManagementElementHash>::iterator> ret;

    this->_mutex.get_shared();
    ret = this->_bucketMap.equal_range(hashNum);

    sourEle = BSONElement(eleHash.data);

    for(std::multimap<unsigned int, IndexManagementElementHash>::iterator it = ret.first; it != ret.second; ++it) {
        existEle = it->second;
        destEle = BSONElement(existEle.data);

        if(sourEle.type() == destEle.type()) {
            if(sourEle.valuesize() == destEle.valuesize()) {
                if(!memcmp(sourEle.value(), destEle.value(), destEle.valuesize())) {
                    rc = DB_INDEX_MANAGEMENT_ID_EXIST;
                    PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "record _id does exist");
                    goto error;
                }
            }
        }
    }

    done:
        this->_mutex.release_shared ();
        return rc;
    error:
        goto done;
}

int IndexManagementBucketManager::IndexManagementBucket::createIndex(unsigned int hashNum,
                                                                     IndexManagementElementHash &eleHash) {
    int rc = DB_OK ;
    this->_mutex.get() ;
    this->_bucketMap.insert(pair<unsigned int, IndexManagementElementHash>(hashNum, eleHash));
    this->_mutex.release();
    return rc ;
}

int IndexManagementBucketManager::IndexManagementBucket::findIndex(unsigned int hashNum,
                                                                   IndexManagementElementHash &eleHash)
{
    int rc = DB_OK;

    BSONElement destEle;
    BSONElement sourEle;

    IndexManagementElementHash existEle;

    std::pair<std::multimap<unsigned int, IndexManagementElementHash>::iterator, std::multimap<unsigned int, IndexManagementElementHash>::iterator> ret;

    this->_mutex.get_shared();

    ret = this->_bucketMap.equal_range(hashNum);

    sourEle = BSONElement(eleHash.data);

    for(std::multimap<unsigned int, IndexManagementElementHash>::iterator it = ret.first; it != ret.second; ++it) {
        existEle = it->second;
        destEle = BSONElement(existEle.data);

        if(sourEle.type() == destEle.type()) {
            if(sourEle.valuesize() == destEle.valuesize()) {
                if(!memcmp(sourEle.value(), destEle.value(), destEle.valuesize())) {
                    eleHash.recordId = existEle.recordId ;
                    goto done ;
                }
            }
        }
    }

    rc = DB_INDEX_MANAGEMENT_ID_NOT_EXIST ;
    PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "record _id does not exist, hashNum = %d", hashNum) ;

    goto error;

    done:
        this->_mutex.release_shared ();
        return rc;
    error:
        goto done;
}

int IndexManagementBucketManager::IndexManagementBucket::removeIndex(unsigned int hashNum,
                                                                     IndexManagementElementHash &eleHash) {
    int rc = DB_OK;

    BSONElement destEle;
    BSONElement sourEle;

    IndexManagementElementHash existEle;

    std::pair<std::multimap<unsigned int, IndexManagementElementHash>::iterator, std::multimap<unsigned int, IndexManagementElementHash>::iterator> ret;

    this->_mutex.get();

    ret = this->_bucketMap.equal_range(hashNum);

    sourEle = BSONElement(eleHash.data) ;
    for(std::multimap<unsigned int, IndexManagementElementHash>::iterator it = ret.first; it != ret.second; ++it) {
        existEle = it->second;
        destEle = BSONElement(existEle.data);

        if(sourEle.type() == destEle.type()) {
            if(sourEle.valuesize() == destEle.valuesize()) {
                if(!memcmp(sourEle.value(), destEle.value(), destEle.valuesize())) {
                    eleHash.recordId = existEle.recordId;
                    this->_bucketMap.erase(it);
                    goto done;
                }
            }
        }
    }

    rc = DB_INVALID_ARG;
    PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "record _id does not exist");

    goto error;

    done:
        this->_mutex.release () ;
        return rc;
    error:
        goto done;
}

int IndexManagementBucketManager::initialize () {
    int rc = DB_OK;
    IndexManagementBucket *temp = NULL;

    for(int i = 0; i < INDEX_MANAGEMENT_HASH_MAP_SIZE; ++i) {
        temp = new (std::nothrow) IndexManagementBucket();
        if(!temp) {
            rc = DB_OOM;
            PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "Failed to allocate new IndexManagementBucket");
            goto error;
        }

        this->_bucket.push_back(temp) ;
        temp = NULL ;
    }

    done:
        return rc;
    error :
        goto done;
}

//IndexManagementBucketManager::IndexManagementBucketManager() = default;

IndexManagementBucketManager::~IndexManagementBucketManager() {
    IndexManagementBucket *pIndexManagementBucket = NULL;

    for (int i = 0; i < INDEX_MANAGEMENT_HASH_MAP_SIZE; ++i) {
        pIndexManagementBucket = this->_bucket[i];
        if(pIndexManagementBucket) {
            delete pIndexManagementBucket;
        }
    }
}
