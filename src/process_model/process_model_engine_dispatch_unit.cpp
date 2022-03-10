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

#include "process_model_engine_dispatch_unit_manager.hpp"
#include "process_model_engine_dispatch_unit.hpp"
#include "process_model.hpp"
#include "problem_detect.hpp"
#include "process_model_tcp_listener.hpp"

static std::map<ENGINE_DISPATCH_UNIT_TYPES, std::string> mapEngineDispatchUnitName;
static std::map<ENGINE_DISPATCH_UNIT_TYPES, ENGINE_DISPATCH_UNIT_TYPES> mapEngineDispatchUnitTypeSys;

int registerEngineDispatchUnitName(ENGINE_DISPATCH_UNIT_TYPES type, const char *name, bool system) {
    int rc = DB_OK;

    std::map<ENGINE_DISPATCH_UNIT_TYPES, std::string>::iterator it = mapEngineDispatchUnitName.find(type);

    if (it != mapEngineDispatchUnitName.end()) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "EDU Type conflict[type:%d, %s<->%s]",
                            (int) type, it->second.c_str(), name);
        rc = DB_SYS;
        goto error;
    }

    mapEngineDispatchUnitName[type] = std::string(name);

    if (system) {
        mapEngineDispatchUnitTypeSys[type] = type;
    }

    done :
        return rc;
    error :
        goto done;
}

const char *getEDUName(ENGINE_DISPATCH_UNIT_TYPES type) {
    std::map<ENGINE_DISPATCH_UNIT_TYPES, std::string>::iterator it = mapEngineDispatchUnitName.find(type);

    if (it != mapEngineDispatchUnitName.end()) {
        return it->second.c_str();
    }

    return "Unknown";
}

bool isSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPES type) {
    std::map<ENGINE_DISPATCH_UNIT_TYPES, ENGINE_DISPATCH_UNIT_TYPES>::iterator it = mapEngineDispatchUnitTypeSys.find(type);
    return !(it == mapEngineDispatchUnitTypeSys.end());
}

ProcessModelEngineDispatchUnitControlBlock::ProcessModelEngineDispatchUnitControlBlock(
        ProcessModelEngineDispatchUnitManager *mgr, ENGINE_DISPATCH_UNIT_TYPES type) :
        _type(type),
        _mgr(mgr),
        _status(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING),
        _id(0),
        _isForced(false),
        _isDisconnected(false) {
}

//inline void ProcessModelEngineDispatchUnitControlBlock::postEvent(const ProcessModelEngineDispatchUnitEvent &data) {
//    this->_queue.push(data);
//}

bool ProcessModelEngineDispatchUnitControlBlock::waitEvent(ProcessModelEngineDispatchUnitEvent &data,
                                                           long long int millsec) {
    bool waitMsg = false ;
    if ( PROCESS_MODEL_ENGINE_DISPATCH_UNIT_IDLE != this->_status ) {
        this->_status = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_WAITING;
    }

    if ( 0 > millsec ) {
        this->_queue.wait_and_pop(data);
        waitMsg = true ;
    } else {
        waitMsg = this->_queue.time_wait_and_pop(data, millsec);
    }

    if ( waitMsg ) {
        if ( data._eventType == PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_TERM ) {
            this->_isDisconnected = true ;
        } else {
            this->_status = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_RUNNING ;
        }
    }

    return waitMsg ;
}

//inline void ProcessModelEngineDispatchUnitControlBlock::force() {
//    this->_isForced = true;
//}

//inline void ProcessModelEngineDispatchUnitControlBlock::disconnect() {
//    this->_isDisconnected = true;
//}

//inline ENGINE_DISPATCH_UNIT_TYPES ProcessModelEngineDispatchUnitControlBlock::getType() {
//    return ENGINE_DISPATCH_UNIT_TYPE_MAXIMUM;
//}

//inline void ProcessModelEngineDispatchUnitControlBlock::setType(ENGINE_DISPATCH_UNIT_TYPES type) {
//    this->_type = type;
//}

//inline ENGINE_DISPATCH_UNIT_ID ProcessModelEngineDispatchUnitControlBlock::getID() {
//    return this->_id;
//}

//inline void ProcessModelEngineDispatchUnitControlBlock::setID(ENGINE_DISPATCH_UNIT_ID id) {
//    this->_id = id;
//}

//inline void ProcessModelEngineDispatchUnitControlBlock::setStatus(ENGINE_DISPATCH_UNIT_STATUS status) {
//    this->_status = status;
//}

//inline ENGINE_DISPATCH_UNIT_STATUS ProcessModelEngineDispatchUnitControlBlock::getStatus() {
//    return this->_status;
//}

//inline bool ProcessModelEngineDispatchUnitControlBlock::isForced() {
//    return this->_isForced;
//}

//ProcessModelEngineDispatchUnitManager *ProcessModelEngineDispatchUnitControlBlock::getEngineDispatchManager() {
//    return this->_mgr;
//}

struct EngineDispatchUnitEntryInfo {
    int regResult;
    int type;
    ProcessModelEntryPoint entryFunc;
};

#define ON_ENGINE_DISPATCH_UNIT_TYPE_TO_ENTRY1(type, system, entry, desp) \
   { type, registerEngineDispatchUnitName(type, desp, system), entry }

ProcessModelEntryPoint getEntryFuncByType(ENGINE_DISPATCH_UNIT_TYPES type) {
    ProcessModelEntryPoint rt = NULL;

    static const EngineDispatchUnitEntryInfo entry[] = {
            ON_ENGINE_DISPATCH_UNIT_TYPE_TO_ENTRY1 (ENGINE_DISPATCH_UNIT_TYPE_AGENT, false,
                                                    processModelAgentEntryPoint, "Agent"),
            ON_ENGINE_DISPATCH_UNIT_TYPE_TO_ENTRY1 (ENGINE_DISPATCH_UNIT_TYPE_TCP_LISTENER, true,
                                                    processModelTcpListenerEntryPoint, "TCPListener"),
            ON_ENGINE_DISPATCH_UNIT_TYPE_TO_ENTRY1 (ENGINE_DISPATCH_UNIT_TYPE_MAXIMUM, false, NULL, "Unknown")
    };

    // entry[] element number
    static const unsigned int number = 3;

    for (unsigned  int index = 0; index < number; ++index) {
        if (entry[index].type == type) {
            rt = entry[index].entryFunc;
            goto done;
        }
    }

    done :
        return rt;
}

int processModelRecv(char *pBuffer, int recvSize, OSServiceSocket *sock, ProcessModelEngineDispatchUnitControlBlock *cb) {
    int rc = DB_OK;

    DB_ASSERT (sock, "Socket is NULL");
    DB_ASSERT (cb, "cb is NULL");

    while (true) {
        if (cb->isForced()) {
            rc = DB_APP_FORCED;
            goto done;
        }

        rc = sock->recv(pBuffer, recvSize);
        if (DB_TIMEOUT == rc) {
            continue;
        }
        goto done;
    }

    done :
        return rc;
}

int processModelSend(const char *pBuffer, int sendSize, OSServiceSocket *sock, ProcessModelEngineDispatchUnitControlBlock *cb) {
    int rc = DB_OK;

    DB_ASSERT (sock, "Socket is NULL");
    DB_ASSERT (cb, "cb is NULL");

    while (true) {
        if (cb->isForced()) {
            rc = DB_APP_FORCED;
            goto done;
        }

        rc = sock->send(pBuffer, sendSize);
        if (DB_TIMEOUT == rc)
            continue;
        goto done;
    }

    done :
        return rc;
}

int processModelEngineDispatchUnitEntryPoint(ENGINE_DISPATCH_UNIT_TYPES type, ProcessModelEngineDispatchUnitControlBlock *cb, void *arg) {
    int rc = DB_OK;

    ENGINE_DISPATCH_UNIT_ID myEngineDispatchUnitID = cb->getID();
    ProcessModelEngineDispatchUnitManager *eduMgr = cb->getEngineDispatchManager();
    ProcessModelEngineDispatchUnitEvent event;

    bool eduDestroyed = false;
    bool isForced = false;

    while (!eduDestroyed) {
        type = cb->getType();
        if (!cb->waitEvent(event, 100)) {
            if (cb->isForced()) {
                PROBLEM_DETECT_LOG (PROBLEM_DETECT_EVENT, "EDU %lld is forced", myEngineDispatchUnitID);
                isForced = true;
            } else
                continue;
        }
        if (!isForced && PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_RESUME == event._eventType) {

            eduMgr->waitEngineDispatchUnit(myEngineDispatchUnitID);

            ProcessModelEntryPoint entryFunc = getEntryFuncByType(type);
            if (!entryFunc) {
                PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "EDU %lld type %d entry point func is NULL",
                                    myEngineDispatchUnitID, type);
                SHUTDOWN_DB
                rc = DB_SYS;
            } else {
                rc = entryFunc(cb, event.Data);
            }

            if (IS_DB_UP) {

                if (isSystemEngineDispatchUnit(cb->getType())) {
                    PROBLEM_DETECT_LOG (PROBLEM_DETECT_SEVERE, "System EDU: %lld, type %s exits with %d",
                                        myEngineDispatchUnitID, getEDUName(type), rc);
                    SHUTDOWN_DB
                } else if (rc) {
                    PROBLEM_DETECT_LOG (PROBLEM_DETECT_WARNING, "EDU %lld, type %s, exits with %d",
                                        myEngineDispatchUnitID, getEDUName(type), rc);
                }
            }
            eduMgr->waitEngineDispatchUnit(myEngineDispatchUnitID);
        } else if (!isForced && PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_TERM != event._eventType) {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Receive the wrong event %d in EDU %lld, type %s",
                                event._eventType, myEngineDispatchUnitID, getEDUName(type));
            rc = DB_SYS;
        } else if (isForced && PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_TERM == event._eventType && cb->isForced()) {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_EVENT, "EDU %lld, type %s is forced", myEngineDispatchUnitID, type);
            isForced = true;
        }

        if (!isForced && event.Data && event._release) {
            free(event.Data);
            event.reset();
        }

        rc = eduMgr->returnEngineDispatchUnit(myEngineDispatchUnitID, isForced, &eduDestroyed);
        if (rc) {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid EDU Status for EDU: %lld, type %s",
                                myEngineDispatchUnitID, getEDUName(type));
        }
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_DEBUG, "Terminating thread for EDU: %lld, type %s",
                            myEngineDispatchUnitID, getEDUName(type));
    }
    return 0;
}
