/*******************************************************************************
                  Copyright (C) 2021 BerryDB Software Inc.
This application is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License, Version 3, as
published by the Free Software Foundation.

This application is distributed in the hope that i
 t will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR PARTICULAR PURPOSE, See the GNU Affero General Public License for more
details.

You should have received a copy of the GNU Affero General Public License along
with this application. If not, see <http://www.gnu.org/license/>
*******************************************************************************/

#ifndef _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_HPP__
#define _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_HPP__

#include "core.hpp"
#include "process_model_engine_dispatch_unit_event.hpp"
#include "os_service_queue.hpp"
#include "os_service_socket.hpp"

#define PROCESS_MODEL_INVALID_ENGINE_DISPATCH_UNIT_ID        0
#define PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_CREATING(x)    ( PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING == x )
#define PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_RUNNING(x)     ( PROCESS_MODEL_ENGINE_DISPATCH_UNIT_RUNNING  == x )
#define PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_WAITING(x)     ( PROCESS_MODEL_ENGINE_DISPATCH_UNIT_WAITING  == x )
#define PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_IDLE(x)        ( PROCESS_MODEL_ENGINE_DISPATCH_UNIT_IDLE     == x )
#define PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_DESTROY(x)     ( PROCESS_MODEL_ENGINE_DISPATCH_UNIT_DESTROY  == x )

typedef unsigned long long ENGINE_DISPATCH_UNIT_ID;

enum ENGINE_DISPATCH_UNIT_TYPES {
    ENGINE_DISPATCH_UNIT_TYPE_TCP_LISTENER = 0,
    ENGINE_DISPATCH_UNIT_TYPE_AGENT,
    ENGINE_DISPATCH_UNIT_TYPE_UNKNOWN,
    ENGINE_DISPATCH_UNIT_TYPE_MAXIMUM = ENGINE_DISPATCH_UNIT_TYPE_UNKNOWN
};

enum ENGINE_DISPATCH_UNIT_STATUS {
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING = 0,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_RUNNING,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_WAITING,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_IDLE,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_DESTROY,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_UNKNOWN,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_STATUS_MAXIMUM = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_UNKNOWN
};

class ProcessModelEngineDispatchUnitManager;

class ProcessModelEngineDispatchUnitControlBlock {
public:
    ProcessModelEngineDispatchUnitControlBlock(ProcessModelEngineDispatchUnitManager* mgr, ENGINE_DISPATCH_UNIT_TYPES t);
    inline void postEvent(ProcessModelEngineDispatchUnitEvent const &data) { this->_queue.push(data); }
    bool waitEvent(ProcessModelEngineDispatchUnitEvent &data, long long millsec);
    inline void force() { this->_isForced = true; }
    inline void disconnect() { this->_isDisconnected = true; }
    inline ENGINE_DISPATCH_UNIT_TYPES getType() { return ENGINE_DISPATCH_UNIT_TYPE_MAXIMUM; }
    inline void setType(ENGINE_DISPATCH_UNIT_TYPES type) { this->_type = type; }
    inline ENGINE_DISPATCH_UNIT_ID getID() { return this->_id; }
    inline void setID(ENGINE_DISPATCH_UNIT_ID id) { this->_id = id; }
    inline void setStatus(ENGINE_DISPATCH_UNIT_STATUS status) { this->_status = status; }
    inline ENGINE_DISPATCH_UNIT_STATUS getStatus() { return this->_status; }
    inline bool isForced() { return this->_isForced; }
    inline ProcessModelEngineDispatchUnitManager* getEngineDispatchManager() { return this->_mgr; }

private:
    ENGINE_DISPATCH_UNIT_TYPES _type;
    ProcessModelEngineDispatchUnitManager* _mgr;
    ENGINE_DISPATCH_UNIT_STATUS _status;
    ENGINE_DISPATCH_UNIT_ID _id;
    bool _isForced;
    bool _isDisconnected;
    OSServiceQueue<ProcessModelEngineDispatchUnitEvent> _queue;
};

typedef int (*ProcessModelEntryPoint) (ProcessModelEngineDispatchUnitControlBlock* krcb, void* p);

ProcessModelEntryPoint getEntryFuncByType(ENGINE_DISPATCH_UNIT_TYPES type);

int processModelAgentEntryPoint(ProcessModelEngineDispatchUnitControlBlock* krcb, void* arg);
int processModelEngineDispatchUnitEntryPoint(ENGINE_DISPATCH_UNIT_TYPES type,
                                             ProcessModelEngineDispatchUnitControlBlock* krcb, void* arg);
int processModelRecv(char* pBuff, int recvSize, OSServiceSocket* sock, ProcessModelEngineDispatchUnitControlBlock* krcb);
int processModelSend(char* pBuff, int sendSize, OSServiceSocket* sock, ProcessModelEngineDispatchUnitControlBlock* krcb);

#endif
