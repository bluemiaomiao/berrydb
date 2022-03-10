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

#include "process_model.hpp"
#include "process_model_options.hpp"
#include "problem_detect.hpp"

DBKernelControlBlock processModelKernelControlBlock;

extern char problemDetectDiagLogPath[OS_SERVICE_MAX_PATHSIZE + 1];

int DBKernelControlBlock::init(ProcessModelOptions *options) {
    setDBStatus(DB_NORMAL);
    setDataFilePath(options->getDBPath());
    setLogFilePath(options->getLogPath());
    strncpy(problemDetectDiagLogPath, getLogFilePath(), sizeof(problemDetectDiagLogPath));
    setServiceName(options->getServiceName());
    setMaxPool(options->getMaxPool());
    return _rtnMgr.runtimeInitialize();
}

DBKernelControlBlock::DBKernelControlBlock() {
    this->_dbStatus = DB_NORMAL;
    memset(this->_dataFilePath, 0, sizeof(this->_dataFilePath));
    memset(this->_logFilePath, 0, sizeof(this->_logFilePath));
    this->_maxPool = 0;
    memset(this->_svcName, 0, sizeof(this->_svcName));
}

//DBKernelControlBlock::~DBKernelControlBlock() = default;

ProcessModelEngineDispatchUnitManager *DBKernelControlBlock::getEngineDispatchUnitManager() {
    return &this->_eduMgr;
}

Runtime *DBKernelControlBlock::getRuntimeManager() {
    return &this->_rtnMgr;
}

//DB_STATUS DBKernelControlBlock::getDBStatus() {
//    return this->_dbStatus;
//}

//const char *DBKernelControlBlock::getDataFilePath() {
//    return this->_dataFilePath;
//}

//const char *DBKernelControlBlock::getLogFilePath() {
//    return this->_logFilePath;
//}

//const char *DBKernelControlBlock::getServiceName() {
//    return this->_svcName;
//}

//int DBKernelControlBlock::getMaxPool() {
//    return this->_maxPool;
//}

//void DBKernelControlBlock::setDBStatus(DB_STATUS status) {
//    this->_dbStatus = status;
//}

void DBKernelControlBlock::setDataFilePath(const char *pPath) {
    strncpy ( this->_dataFilePath, pPath, sizeof(this->_dataFilePath) ) ;
}

void DBKernelControlBlock::setServiceName(const char *pName) {
    strncpy(this->_svcName, pName, sizeof( this->_svcName));
}

void DBKernelControlBlock::setMaxPool(int maxPool) {
    this->_maxPool = maxPool;
}

void DBKernelControlBlock::setLogFilePath(const char * pPath) {
    strncpy(this->_logFilePath, pPath, sizeof(this->_logFilePath));
}

//inline DBKernelControlBlock* processModelGetKernelControlBlock() {
//    return &processModelKernelControlBlock;
//}