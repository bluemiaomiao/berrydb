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

#ifndef _INDEX_MANAGEMENT_BUCKET_HPP__
#define _INDEX_MANAGEMENT_BUCKET_HPP__

#include "os_service_latch.hpp"
#include "bson.h"
#include <map>
#include "data_management_record.hpp"

using namespace bson;

#define INDEX_MANAGEMENT_KEY_FIELDNAME "_id"
#define INDEX_MANAGEMENT_HASH_MAP_SIZE 1000

struct IndexManagementElementHash {
    const char *data;
    DataManagementRecordID recordId;
};

class IndexManagementBucketManager {
private:
    class IndexManagementBucket {
    private:
        std::multimap<unsigned int, IndexManagementElementHash> _bucketMap;
        OSServiceSLatch _mutex;
    public:
        int isIDExist(unsigned int hashNum, IndexManagementElementHash &eleHash);
        int createIndex(unsigned int hashNum, IndexManagementElementHash &eleHash);
        int findIndex(unsigned int hashNum, IndexManagementElementHash &eleHash);
        int removeIndex(unsigned int hashNum, IndexManagementElementHash &eleHash);
    };
    int _processData(BSONObj &record, DataManagementRecordID &recordID, unsigned int &hashNum,
                     IndexManagementElementHash &eleHash, unsigned int &random);
    std::vector<IndexManagementBucket*> _bucket;
public:
    IndexManagementBucketManager();
    ~IndexManagementBucketManager();
    int initialize();
    int isIDExist(BSONObj &record);
    int createIndex(BSONObj &record, DataManagementRecordID &recordID);
    int findIndex(BSONObj &record, DataManagementRecordID &recordID);
    int removeIndex(BSONObj &record, DataManagementRecordID &recordID);
};

#endif