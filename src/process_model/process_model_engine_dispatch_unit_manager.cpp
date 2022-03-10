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

#include "problem_detect.hpp"
#include "process_model.hpp"
#include "process_model_engine_dispatch_unit_manager.hpp"

int ProcessModelEngineDispatchUnitManager::_destroyAll() {
    this->_setDestroyed(true);
    this->setQuiesced(true);
    
    unsigned int timeCounter = 0;
    unsigned int eduCount = this->_getEngineDispatchUnitCount(ENGINE_DISPATCH_UNIT_USER);

    while(eduCount != 0) {
        if(0 == timeCounter % 50) {
            this->_forceEngineDispatchUnits(ENGINE_DISPATCH_UNIT_USER);
        }
        
        ++timeCounter;
        OSServiceSleepMillis(100);
        eduCount = this->_getEngineDispatchUnitCount(ENGINE_DISPATCH_UNIT_USER);
    }
    
    timeCounter = 0;
    eduCount = this->_getEngineDispatchUnitCount(ENGINE_DISPATCH_UNIT_ALL);
    
    while(eduCount != 0){
        if(0 == timeCounter % 50){
            this->_forceEngineDispatchUnits(ENGINE_DISPATCH_UNIT_ALL);
        }

        ++timeCounter;
        OSServiceSleepMillis(100);
        eduCount = this->_getEngineDispatchUnitCount(ENGINE_DISPATCH_UNIT_ALL);
    }

    return DB_OK;
}

int ProcessModelEngineDispatchUnitManager::forceUserEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID eduID) {
    int rc = DB_OK;
    std::map<ENGINE_DISPATCH_UNIT_ID , ProcessModelEngineDispatchUnitControlBlock*>::iterator it;
    this->_mutex.get();

    if(this->isSystemEngineDispatchUnit(eduID)) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "System engine dispatch unit %d can't be forced", eduID);
        rc = DB_PROCESS_MODEL_FORCE_SYS_ENGINE_DISPATCH_UNIT;
        goto error;
    }

    for(it = this->_runQueue.begin(); it != this->_runQueue.end(); ++it) {
        if((*it).second->getID() == eduID) {
            (*it).second->force();
            goto done;
        }
    }

    for(it = this->_idleQueue.begin(); it != this->_idleQueue.end(); ++it) {
        if((*it).second->getID() == eduID) {
            (*it).second->force();
            goto done;
        }
    }

    done :
        this->_mutex.release();
        return rc;
    error :
        goto done;
}

int ProcessModelEngineDispatchUnitManager::_forceEngineDispatchUnits(int property) {
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get();

    for(it = this->_runQueue.begin(); it != this->_runQueue.end(); ++it) {
        if(((ENGINE_DISPATCH_UNIT_SYSTEM & property) && _isSystemEngineDispatchUnit(it->first))
             || ((ENGINE_DISPATCH_UNIT_USER & property) && !_isSystemEngineDispatchUnit(it->first))) {
           (*it).second->force();
            PROBLEM_DETECT_LOG(PROBLEM_DETECT_DEBUG, "force engine dispatch unit[ID:%lld]", it->first);
        }
    }

    for(it = this->_idleQueue.begin(); it != this->_idleQueue.end(); ++it) {
        if(ENGINE_DISPATCH_UNIT_USER & property) {
           (*it).second->force();
        }
    }

    this->_mutex.release();

    return DB_OK;
}

unsigned int ProcessModelEngineDispatchUnitManager::_getEngineDispatchUnitCount(int property) {
    unsigned int eduCount = 0;

    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get_shared();

    for(it = this->_runQueue.begin(); it != this->_runQueue.end(); ++it) {
        if(((ENGINE_DISPATCH_UNIT_SYSTEM & property) && _isSystemEngineDispatchUnit(it->first))
             || ((ENGINE_DISPATCH_UNIT_USER & property) && !_isSystemEngineDispatchUnit(it->first))) {
            ++eduCount;
        }
    }

    for(it = this->_idleQueue.begin(); it != this->_idleQueue.end(); ++it) {
        if(ENGINE_DISPATCH_UNIT_USER & property) {
            ++eduCount;
        }
    }

    this->_mutex.release_shared();

    return eduCount;
}

int ProcessModelEngineDispatchUnitManager::postEngineDispatchUnitPost(
        ENGINE_DISPATCH_UNIT_ID eduID, ProcessModelEngineDispatchUnitEventTypes type, bool release , void *pData) {
    
    int rc = DB_OK;
    
    ProcessModelEngineDispatchUnitControlBlock* educb = NULL;
    
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get_shared();

    if(this->_runQueue.end() ==(it = this->_runQueue.find(eduID))) {
        if(this->_idleQueue.end() ==(it = this->_idleQueue.find(eduID))) {
            rc = DB_SYS;
            goto error;
        }
    }

    educb =(*it).second;
    educb->postEvent(ProcessModelEngineDispatchUnitEvent(type, release, pData));

    done :
        this->_mutex.release_shared();
        return rc;
    error :
        goto done;
}

int ProcessModelEngineDispatchUnitManager::waitEngineDispatchUnitPost(
        ENGINE_DISPATCH_UNIT_ID eduID, ProcessModelEngineDispatchUnitEvent& event, long long millsecond = -1) {

    int rc = DB_OK;

    ProcessModelEngineDispatchUnitControlBlock* eduCB = NULL;

    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get_shared();
    if(this->_runQueue.end() ==(it = this->_runQueue.find(eduID))) {
        if(this->_idleQueue.end() ==(it =this-> _idleQueue.find(eduID))) {
            rc = DB_SYS;
            goto error;
        }
    }

    eduCB =(*it).second;

    if(!eduCB->waitEvent(event, millsecond)) {
        rc = DB_TIMEOUT;
        goto error;
    }

    done :
        this->_mutex.release_shared();
        return rc;
    error :
        goto done;
}

int ProcessModelEngineDispatchUnitManager::returnEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID eduID, bool force, bool* destroyed) {
    int rc = DB_OK;

    ENGINE_DISPATCH_UNIT_TYPES type  = ENGINE_DISPATCH_UNIT_TYPE_UNKNOWN;

    ProcessModelEngineDispatchUnitControlBlock *educb = NULL;

    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get_shared();

    if(this->_runQueue.end() ==(it = this->_runQueue.find(eduID))) {
        if(this->_idleQueue.end() ==(it = this->_idleQueue.find(eduID))) {
            rc = DB_SYS;
            *destroyed = false;
            this->_mutex.release_shared();
            goto error;
        }
    }
    educb = (*it).second;

    if(educb) {
        type = educb->getType();
    }

    this->_mutex.release_shared();

    if(!ProcessModelEngineDispatchUnitManager::isPoolable(type) || force || isDestroyed() ||
    size() > (unsigned int)processModelGetKernelControlBlock()->getMaxPool()) {

        rc = _destroyEngineDispatchUnit(eduID);
        if(destroyed) {
            if(DB_OK == rc || DB_SYS == rc) {
                *destroyed = true;
            }
            else {
                *destroyed = false;
            }
        }
    } else {
        rc = _destroyEngineDispatchUnit(eduID);

        if(destroyed) {
            if(DB_SYS == rc)
                *destroyed = true;
            else
                *destroyed = false;
        }
    }

    done :
        return rc;
    error :
        goto done;
}

int ProcessModelEngineDispatchUnitManager::startEngineDispatchUnit (ENGINE_DISPATCH_UNIT_TYPES type, void* arg, ENGINE_DISPATCH_UNIT_ID *eduid)
{
    int rc = DB_OK;
    ENGINE_DISPATCH_UNIT_ID eduID = 0;
    ProcessModelEngineDispatchUnitControlBlock* educb = NULL;
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    if(this->isQuiesced()) {
        rc = DB_QUIESCED;
        goto done;
    }

    this->_mutex.get();

    if(this->_idleQueue.empty() || !ProcessModelEngineDispatchUnitManager::isPoolable(type)) {
        this->_mutex.release();
        rc = this->_createNewEngineDispatchUnit(type, arg, eduid);
        if(DB_OK == rc) {
            goto done;
        }
        goto error;
    }

    for(it = this->_idleQueue.begin(); (this->_idleQueue.end() != it) && (PROCESS_MODEL_ENGINE_DISPATCH_UNIT_IDLE !=(*it).second->getStatus()); it ++);

    if(this->_idleQueue.end() == it) {
        this->_mutex.release();

        rc = this->_createNewEngineDispatchUnit(type, arg, eduid );

        if(DB_OK == rc) {
            goto done;
        }

        goto error;
    }

    eduID =(*it).first;
    educb =(*it).second;
    this->_idleQueue.erase(eduID);
    DB_ASSERT(isPoolable(type), "must be agent")

    educb->setType(type);
    educb->setStatus(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_WAITING);
    this->_runQueue [ eduID ] = educb;
    *eduid = eduID;
    this->_mutex.release();

    educb->postEvent(ProcessModelEngineDispatchUnitEvent(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_RESUME, false, arg));

    done :
        return rc;
    error :
        goto done;
}

int ProcessModelEngineDispatchUnitManager::_createNewEngineDispatchUnit (ENGINE_DISPATCH_UNIT_TYPES type, void* arg, ENGINE_DISPATCH_UNIT_ID *eduid) {
    int rc = DB_OK;

    unsigned int probe = 0;

    ProcessModelEngineDispatchUnitControlBlock *cb = NULL;

    ENGINE_DISPATCH_UNIT_ID myENGINE_DISPATCH_UNIT_ID = 0;

    if(this->isQuiesced()) {
        rc = DB_QUIESCED;
        goto done;
    }

    if(!getEntryFuncByType(type)) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "The edu[type:%d] not exist or function is null", type);
        rc = DB_INVALID_ARG;
        probe = 30;
        goto error;
    }

    cb = new(std::nothrow) ProcessModelEngineDispatchUnitControlBlock(this, type);
    DB_VALIDATE_GOTO_ERROR(cb, DB_OOM, "Out of memory to create agent control block");

    cb->setStatus(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING);

    this->_mutex.get();

    if(this->_runQueue.end() != this->_runQueue.find(this->_ENGINE_DISPATCH_UNIT_ID)) {
        this->_mutex.release();
        rc = DB_SYS;
        probe = 10;
        goto error;
    }

    if(this->_idleQueue.end() != this->_idleQueue.find(this->_ENGINE_DISPATCH_UNIT_ID) ) {
        this->_mutex.release();
        rc = DB_SYS;
        probe = 15;
        goto error;
    }

    cb->setID(this->_ENGINE_DISPATCH_UNIT_ID);
    if(eduid) {
        *eduid = _ENGINE_DISPATCH_UNIT_ID;
    }

    this->_runQueue [ _ENGINE_DISPATCH_UNIT_ID ] =(ProcessModelEngineDispatchUnitControlBlock*) cb;
    myENGINE_DISPATCH_UNIT_ID = _ENGINE_DISPATCH_UNIT_ID;
    ++_ENGINE_DISPATCH_UNIT_ID;
    this->_mutex.release();

    try {
        boost::thread agentThread(processModelEngineDispatchUnitEntryPoint, type, cb, arg);
        agentThread.detach();
    } catch(std::exception e) {
        this->_runQueue.erase(myENGINE_DISPATCH_UNIT_ID);
        rc = DB_SYS;
        probe = 20;
        goto error;
    }

    cb->postEvent(ProcessModelEngineDispatchUnitEvent(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_RESUME, false, arg));

    done :
        return rc;
    error :
        if(cb) {
            delete cb;
        }
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "Failed to create new agent, probe = %d", probe);

        goto done;
}

int ProcessModelEngineDispatchUnitManager::waitEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID eduID)
{
    int rc = DB_OK;
    ProcessModelEngineDispatchUnitControlBlock* eduCB = NULL;
    unsigned int eduStatus = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING;
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get_shared();
    if(this->_runQueue.end() ==(it = this->_runQueue.find(eduID))) {
        rc = DB_SYS;
        goto error;
    }
    eduCB =(*it).second;

    eduStatus = eduCB->getStatus();

    if(PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_WAITING(eduStatus)) {
        goto done;
    }

    if(!PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_RUNNING(eduStatus)) {
        rc = DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS;
        goto error;
    }

    eduCB->setStatus(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_WAITING);

    done :
        this->_mutex.release_shared();
        return rc;
    error :
        goto done;
}

int ProcessModelEngineDispatchUnitManager::_destroyEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID eduID) {
    int rc         = DB_OK;
    unsigned int eduStatus = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING;
    ProcessModelEngineDispatchUnitControlBlock* educb  = NULL;
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get();
    if(this->_runQueue.end() ==(it = this->_runQueue.find(eduID))) {
        if(this->_idleQueue.end() != this->_idleQueue.find(eduID) ) {
            goto done;
        }
        rc = DB_SYS;
        goto error;
    }

    educb =(*it).second;

    eduStatus = educb->getStatus();

    if(PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_IDLE(eduStatus)) {
        goto done;
    }

    if(!PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_WAITING(eduStatus)
    && !PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_CREATING(eduStatus)) {

        rc = DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS;

        goto error;
    }

    DB_ASSERT(isPoolable(educb->getType()), "Only agent can be pooled")

    this->_runQueue.erase(eduID);

    educb->setStatus(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_IDLE);

    this->_idleQueue [ eduID ] = educb;

    done :
        this->_mutex.release();
        return rc;
    error :
        goto done;
}

int ProcessModelEngineDispatchUnitManager::activateEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID eduID) {
    int rc = DB_OK;
    unsigned int eduStatus = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING;
    ProcessModelEngineDispatchUnitControlBlock* educb = NULL;
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;

    this->_mutex.get();

    if(this->_idleQueue.end() ==(it = this->_idleQueue.find(eduID))) {
        if(this->_runQueue.end() ==(it = this->_runQueue.find(eduID))) {
            rc = DB_SYS;
            goto error;
        }
        educb =(*it).second;

        eduStatus = educb->getStatus();

        if(PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_RUNNING(eduStatus)) {
            goto done;
        }

        if(!PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_WAITING(eduStatus)
        && !PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_CREATING(eduStatus)) {

            rc = DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS;

            goto error;
        }

        educb->setStatus(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_RUNNING);
        goto done;
    }

    educb =(*it).second;

    eduStatus = educb->getStatus();

    if(PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_RUNNING(eduStatus)) {
        goto done;
    }

    if(!PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_IDLE(eduStatus)) {
        rc = DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS;
        goto error;
    }

    this->_idleQueue.erase(eduID);

    educb->setStatus(PROCESS_MODEL_ENGINE_DISPATCH_UNIT_RUNNING);

    this->_runQueue [ eduID ] = educb;

    done :
        this->_mutex.release();
        return rc;
    error :
        goto done;
}

ProcessModelEngineDispatchUnitControlBlock *ProcessModelEngineDispatchUnitManager::getEngineDispatchUnit(unsigned int tid) {
    std::map<unsigned int, ENGINE_DISPATCH_UNIT_ID>::iterator it;

    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it1;

    ENGINE_DISPATCH_UNIT_ID eduid;

    ProcessModelEngineDispatchUnitControlBlock *pResult = NULL;

    this->_mutex.get_shared();

    it = this->_tid_engine_dispatch_unit_map.find(tid);
    if(this->_tid_engine_dispatch_unit_map.end() == it) {
        pResult = NULL;
        goto done;
    }

    eduid = (*it).second;

    it1 = this->_runQueue.find(eduid);
    if(this->_runQueue.end() != it1) {
        pResult = (*it1).second;
        goto done;
    }

    it1 = this->_idleQueue.find(eduid);
    if(this->_idleQueue.end() != it1) {
        pResult = (*it1).second;
        goto done;
    }

    done :
        this->_mutex.release_shared();

    return pResult;
}

void ProcessModelEngineDispatchUnitManager::setEngineDispatchUnit(unsigned int tid, ENGINE_DISPATCH_UNIT_ID eduid) {
    this->_mutex.get();
    this->_tid_engine_dispatch_unit_map [ tid ] = eduid;
    this->_mutex.release();
}

ProcessModelEngineDispatchUnitControlBlock *ProcessModelEngineDispatchUnitManager::getEngineDispatchUnit() {
    return getEngineDispatchUnit(OSServiceGetCurrentThreadID());
}

ProcessModelEngineDispatchUnitControlBlock *ProcessModelEngineDispatchUnitManager::getEngineDispatchUnitByID(ENGINE_DISPATCH_UNIT_ID eduID) {
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*>::iterator it;
    ProcessModelEngineDispatchUnitControlBlock *pResult = NULL;

    this->_mutex.get_shared();

    if(this->_runQueue.end() ==(it = this->_runQueue.find(eduID))) {
        if(this->_idleQueue.end() ==(it = this->_idleQueue.find(eduID))) {
            goto done;
        }
    }

    pResult = it->second;

    done :
        this->_mutex.release_shared();

    return pResult;
}

void ProcessModelEngineDispatchUnitManager::_setDestroyed(bool b) {
    this->_isDestroyed = b;
}

bool ProcessModelEngineDispatchUnitManager::_isSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id) {
    std::map<unsigned int, ENGINE_DISPATCH_UNIT_ID>::iterator it = this->_mapSystemEngineDispatchUnits.begin();

    while ( it != this->_mapSystemEngineDispatchUnits.end() ) {
        if ( id == it->second ) {
            return true ;
        }
        ++it ;
    }

    return false ;
}

int ProcessModelEngineDispatchUnitManager::_deactivateEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id) {
    int rc = DB_OK ;

    unsigned int eduStatus = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_CREATING;

    ProcessModelEngineDispatchUnitControlBlock* educb  = NULL ;

    std::map<ENGINE_DISPATCH_UNIT_ID , ProcessModelEngineDispatchUnitControlBlock*>::iterator it ;

    this->_mutex.get() ;
    if ( this->_runQueue.end () == ( it = this->_runQueue.find ( id )) ) {
        if ( this->_idleQueue.end() != this->_idleQueue.find ( id ) ) {
            goto done ;
        }

        rc = DB_SYS ;
        goto error ;
    }

    educb = ( *it ).second ;

    eduStatus = educb->getStatus () ;

    if ( PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_IDLE( eduStatus ) ) {
        goto done ;
    }

    if ( !PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_IDLE( eduStatus )
    && !PROCESS_MODEL_IS_ENGINE_DISPATCH_UNIT_CREATING( eduStatus ) ) {

        rc = DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS ;
        goto error ;
    }

    DB_ASSERT (isPoolable (educb->getType() ), "Only agent can be pooled" )

    this->_runQueue.erase ( id ) ;

    educb->setStatus (PROCESS_MODEL_ENGINE_DISPATCH_UNIT_IDLE ) ;

    this->_idleQueue [ id ] = educb ;

    done :
        this->_mutex.release () ;
        return rc ;
    error :
        goto done ;
}

ProcessModelEngineDispatchUnitManager::~ProcessModelEngineDispatchUnitManager() {
    this->_ENGINE_DISPATCH_UNIT_ID = 1;
    this->_isQuiesced = false;
    this->_isDestroyed = false;
}

void ProcessModelEngineDispatchUnitManager::reset() {
    this->_destroyAll();
}

unsigned int ProcessModelEngineDispatchUnitManager::size() {
    unsigned int num = 0 ;

    this->_mutex.get_shared () ;

    num = ( unsigned int ) this->_runQueue.size() + ( unsigned int ) this->_idleQueue.size () ;

    this->_mutex.release_shared () ;

    return num ;
}

unsigned int ProcessModelEngineDispatchUnitManager::sizeRun() {
    unsigned int num = 0 ;

    this->_mutex.get_shared () ;

    num = ( unsigned int ) this->_runQueue.size () ;

    this->_mutex.release_shared () ;

    return num ;
}

unsigned int ProcessModelEngineDispatchUnitManager::sizeIdle() {
    unsigned int num = 0 ;

    this->_mutex.get_shared () ;

    num = ( unsigned int ) this->_idleQueue.size () ;

    this->_mutex.release_shared () ;

    return num ;
}

unsigned int ProcessModelEngineDispatchUnitManager::sizeSystem() {
    unsigned int num = 0 ;

    this->_mutex.get_shared () ;

    num = this->_mapSystemEngineDispatchUnits.size() ;

    this->_mutex.release_shared () ;

    return num ;
}

ENGINE_DISPATCH_UNIT_ID
ProcessModelEngineDispatchUnitManager::getSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPES type) {
    ENGINE_DISPATCH_UNIT_ID eduid = PROCESS_MODEL_INVALID_ENGINE_DISPATCH_UNIT_ID;

    this->_mutex.get_shared () ;

    std::map<unsigned int, ENGINE_DISPATCH_UNIT_ID>::iterator it = this->_mapSystemEngineDispatchUnits.find(type) ;

    if ( it != this->_mapSystemEngineDispatchUnits.end()) {
        eduid = it->second  ;
    }

    this->_mutex.release_shared();

    return eduid ;
}

bool ProcessModelEngineDispatchUnitManager::isSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id) {
    bool isSys = false;

    this->_mutex.get_shared();

    isSys = this->_isSystemEngineDispatchUnit(id);

    this->_mutex.release_shared();

    return isSys;
}

void ProcessModelEngineDispatchUnitManager::regSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPES type,
                                                                        ENGINE_DISPATCH_UNIT_ID id) {
    this->_mutex.get();

    this->_mapSystemEngineDispatchUnits[type] = id;

    this->_mutex.release();
}

bool ProcessModelEngineDispatchUnitManager::isQuiesced() {
    return this->_isQuiesced;
}

void ProcessModelEngineDispatchUnitManager::setQuiesced(bool b) {
    this->_isQuiesced = b;
}

bool ProcessModelEngineDispatchUnitManager::isDestroyed() {
    return this->_isDestroyed;
}
