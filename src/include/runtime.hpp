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

#ifndef _RUNTIME_HPP__
#define _RUNTIME_HPP__

#include "bson.h"
#include "data_management.hpp"
#include "index_management_bucket.hpp"

#define RUNTIME_FILE_NAME "data.1"

using namespace bson;

class Runtime {
private:
    DataManagementFile *_dataManagementFile;
    IndexManagementBucketManager *_indexManagementBucketManager;
public:
    Runtime();

    ~Runtime();

    int runtimeInitialize();

    int runtimeInsert(bson::BSONObj &record);

    int runtimeFind(bson::BSONObj &inRecord, bson::BSONObj &outRecord);

    int runtimeRemove(bson::BSONObj &record);
};

#endif