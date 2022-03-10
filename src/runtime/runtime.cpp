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
#include "runtime.hpp"
#include "problem_detect.hpp"
#include "process_model.hpp"

Runtime::Runtime() : _dataManagementFile(NULL), _indexManagementBucketManager(NULL) {
}

Runtime::~Runtime() {
    if (this->_indexManagementBucketManager) {
        delete this->_indexManagementBucketManager;
    }
    if (this->_dataManagementFile) {
        delete this->_dataManagementFile;
    }
}

int Runtime::runtimeInitialize() {
    int rc = DB_OK;

    this->_indexManagementBucketManager = new(std::nothrow) IndexManagementBucketManager();

    if (!this->_indexManagementBucketManager) {
        rc = DB_OOM;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to new bucket manager");
        goto error;
    }

    this->_dataManagementFile = new(std::nothrow) DataManagementFile(this->_indexManagementBucketManager);

    if (!this->_dataManagementFile) {
        rc = DB_OOM;
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to new dms file");
        goto error;
    }

    rc = this->_indexManagementBucketManager->initialize();

    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to call bucketMgr initialize, rc = %d", rc);
        goto error;
    }

    rc = this->_dataManagementFile->initialize(processModelGetKernelControlBlock()->getDataFilePath());

    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to call dms initialize, rc = %d", rc);
        goto error;
    }

    done :
        return rc;
    error :
        goto done;
}

int Runtime::runtimeInsert(BSONObj &record) {
    int rc = DB_OK;

    DataManagementRecordID recordID;

    BSONObj outRecord;

    rc = this->_indexManagementBucketManager->isIDExist(record);

    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to call isIDExist, rc = %d", rc);

    rc = this->_dataManagementFile->insert(record, outRecord, recordID);

    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to call dms insert, rc = %d", rc);
        goto error;
    }

    rc = this->_indexManagementBucketManager->createIndex(outRecord, recordID);

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to call ixmCreateIndex, rc = %d", rc);

    done :
        return rc;
    error :
        goto done;
}

int Runtime::runtimeFind(BSONObj &inRecord, BSONObj &outRecord) {
    int rc = DB_OK;

    DataManagementRecordID recordID;

    rc = this->_indexManagementBucketManager->findIndex(inRecord, recordID);

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to call ixm findIndex, rc = %d", rc);

    rc = this->_dataManagementFile->find(recordID, outRecord);

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to call dms find, rc = %d", rc);

    done :
        return rc;
    error :
        goto done;
}

int Runtime::runtimeRemove(BSONObj &record) {
    int rc = DB_OK;

    DataManagementRecordID recordID;

    rc = this->_indexManagementBucketManager->removeIndex(record, recordID);

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to call ixm removeIndex, rc = %d", rc);

    rc = this->_dataManagementFile->remove(recordID);

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to call dms remove, rc = %d", rc);

    done :
        return rc;
    error :
        goto done;
}
