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

#ifndef _DATA_MANAGEMENT_HPP__
#define _DATA_MANAGEMENT_HPP__

#include "os_service_latch.hpp"
#include "os_service_mmap_file.hpp"
#include "bson.h"
#include "data_management_record.hpp"
#include "index_management_bucket.hpp"

#include <vector>

typedef unsigned int SLOTOFF;

#define DATA_MANAGEMENT_EXTEND_SIZE                65536
#define DATA_MANAGEMENT_PAGESIZE                   4194304    // 4MB
#define DATA_MANAGEMENT_MAX_RECORD                 (DATA_MANAGEMENT_PAGESIZE - sizeof(DataManagementHeader) - \
                                                   sizeof(DataManagementRecord) - sizeof(SLOTOFF))
#define DATA_MANAGEMENT_MAX_PAGES                  262144
#define DATA_MANAGEMENT_INVALID_SLOTID             0xFFFFFFFF
#define DATA_MANAGEMENT_INVALID_PAGEID             0xFFFFFFFF
#define DATA_MANAGEMENT_KEY_FIELDDNAME             "_id"

extern const char* gKeyFieldName;

#define DATA_MANAGEMENT_FLAG_NORMAL                0
#define DATA_MANAGEMENT_FLAG_DROPPED               1

struct DataManagementRecord {
    unsigned int _size;
    unsigned int _flag;
    char _data[0];
};

#define DATA_MANAGEMENT_HEADER_EYECATCHER                 "DMSH"
#define DATA_MANAGEMENT_HEADER_EYECATCHER_LEN             4
#define DATA_MANAGEMENT_HEADER_FLAG_NORMAL                0
#define DATA_MANAGEMENT_HEADER_FLAG_DROPPED               1

#define DATA_MANAGEMENT_HEADER_VERSION_0                  0
#define DATA_MANAGEMENT_HEADER_VERSION_CURRENT            DATA_MANAGEMENT_HEADER_VERSION_0

struct DataManagementHeader {
    char _eyeCatcher[DATA_MANAGEMENT_HEADER_EYECATCHER_LEN];
    unsigned int _size;
    unsigned int _flag;
    unsigned int _version;
};

//                          Page Structure
// +-------------------------------------------------------------+
// |                         Page Header                         |
// +-------------------------------------------------------------+
// |                          Slot List                          |
// +-------------------------------------------------------------+
// |                         Free Space                          |
// +-------------------------------------------------------------+
// |                            Data                             |
// +-------------------------------------------------------------+
#define DATA_MANAGEMENT_PAGE_EYECATCHER                 "PAGH"
#define DATA_MANAGEMENT_PAGE_EYECATCHER_LEN             4
#define DATA_MANAGEMENT_PAGE_FLAG_NORMAL                0
#define DATA_MANAGEMENT_PAGE_FLAG_UNALLOC               1
#define DATA_MANAGEMENT_SLOT_EMPTY                      0xFFFFFFFF

struct DataManagementPageHeader {
   char _eyeCatcher[DATA_MANAGEMENT_PAGE_EYECATCHER_LEN];
   unsigned int _size ;
   unsigned int _flag ;
   unsigned int _numSlots ;
   unsigned int _slotOffset ;
   unsigned int _freeSpace ;
   unsigned int _freeOffset ;
   char _data[0] ;
};

#define DATA_MANAGEMENT_FILE_SEGMENT_SIZE           134217728
#define DATA_MANAGEMENT_FILE_HEADER_SIZE            65536
#define DATA_MANAGEMENT_PAGES_PER_SEGMENT           (DATA_MANAGEMENT_FILE_SEGMENT_SIZE / DATA_MANAGEMENT_PAGESIZE)
#define DATA_MANAGEMENT_MAX_SEGMENTS                (DATA_MANAGEMENT_MAX_PAGES / DATA_MANAGEMENT_PAGES__PER_SEGMENT)

class DataManagementFile : public OSServiceMmapFile {
private:
    DataManagementHeader* _header;
    std::vector<char*> _body;
    std::multimap<unsigned int, PAGEID> _freeSpaceMap;
    OSServiceSLatch _mutex;
    OSServiceXLatch _extendMutex;
    char* _pFileName;
    IndexManagementBucketManager* _ixmBucketMgr;
    int _extendSegment();
    int _initNew();
    int _extendFile(int size);
    int _loadData();
    int _searchSlot(char* page, DataManagementRecordID &recordID, SLOTOFF &slot);
    void _recoverSpace(char* page);
    void _updateFreeSpace(DataManagementPageHeader* header, int changeSize, PAGEID pageID);
    PAGEID _findPage(size_t requiredSize);
public:
    DataManagementFile(IndexManagementBucketManager *ixmBucketMgr);
    ~DataManagementFile();
    int initialize(const char *pFileName);
    int insert(bson::BSONObj &record, bson::BSONObj &outRecord, DataManagementRecordID &rid);
    int remove(DataManagementRecordID &rid);
    int find(DataManagementRecordID &rid, bson::BSONObj &result);
    inline unsigned int getNumSegments() { return this->_body.size(); }
    inline unsigned int getNumPages() { return this->getNumSegments() * DATA_MANAGEMENT_PAGES_PER_SEGMENT; }
    inline char* pageToOffset(PAGEID pageID) {
        if(pageID >= this->getNumPages()) {
            return NULL;
        }

        return this->_body[pageID / DATA_MANAGEMENT_PAGES_PER_SEGMENT] + DATA_MANAGEMENT_PAGESIZE * (pageID & DATA_MANAGEMENT_PAGES_PER_SEGMENT);
    }
    inline bool validSize(size_t size) {
        if(size < DATA_MANAGEMENT_FILE_HEADER_SIZE) {
            return false;
        }

        size -= DATA_MANAGEMENT_FILE_SEGMENT_SIZE;

        return (size % DATA_MANAGEMENT_FILE_SEGMENT_SIZE) == 0;
    }
};

#endif