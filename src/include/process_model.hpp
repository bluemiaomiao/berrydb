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

#ifndef _PROCESS_MODEL_HPP__
#define _PROCESS_MODEL_HPP__

#include "core.hpp"
#include "process_model_engine_dispatch_unit_manager.hpp"
#include "runtime.hpp"

enum DB_STATUS {
    DB_NORMAL = 0,
    DB_SHUTDOWN,
    DB_PANIC
};

#define IS_DB_NORMAL ( DB_NORMAL == processModelGetKernelControlBlock()->getDBStatus() )
#define IS_DB_DOWN ( DB_SHUTDOWN == processModelGetKernelControlBlock()->getDBStatus() || \
                   DB_PANIC == processModelGetKernelControlBlock()->getDBStatus() )
#define SHUTDOWN_DB { processModelGetKernelControlBlock()->setDBStatus(DB_SHUTDOWN); }
#define IS_DB_UP ( !IS_DB_DOWN )

class ProcessModelOptions;

class DBKernelControlBlock {
private:
    char _dataFilePath[OS_SERVICE_MAX_PATHSIZE + 1];
    char _logFilePath[OS_SERVICE_MAX_PATHSIZE + 1];
    int _maxPool;
    char _svcName[NI_MAXSERV + 1];
    DB_STATUS _dbStatus;
    ProcessModelEngineDispatchUnitManager _eduMgr;
    Runtime _rtnMgr;
public:
    DBKernelControlBlock();
    ~DBKernelControlBlock() {}
    ProcessModelEngineDispatchUnitManager* getEngineDispatchUnitManager();
    Runtime* getRuntimeManager();
    inline DB_STATUS getDBStatus() { return this->_dbStatus; }
    inline const char* getDataFilePath() { return this->_dataFilePath; }
    inline const char* getLogFilePath() { return this->_logFilePath; }
    inline const char* getServiceName() { return this->_svcName; }
    inline int getMaxPool() { return this->_maxPool; }
    inline void setDBStatus(DB_STATUS status) { this->_dbStatus = status; }
    void setDataFilePath(const char* pPath);
    void setLogFilePath(const char* pPath);
    void setServiceName(const char* pName);
    void setMaxPool(int maxPool);
    int init(ProcessModelOptions* options);
};

extern DBKernelControlBlock processModelKernelControlBlock;

inline DBKernelControlBlock* processModelGetKernelControlBlock() { return &processModelKernelControlBlock; }

#endif