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

#ifndef _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_MANAGER_HPP__
#define _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_MANAGER_HPP__

#include "core.hpp"
#include "process_model_engine_dispatch_unit.hpp"
#include "os_service_latch.hpp"
#include "os_service_util.hpp"

#define ENGINE_DISPATCH_UNIT_SYSTEM      0x01
#define ENGINE_DISPATCH_UNIT_USER        0x02
#define ENGINE_DISPATCH_UNIT_ALL         (ENGINE_DISPATCH_UNIT_SYSTEM | ENGINE_DISPATCH_UNIT_USER)

// +----------------------------------------------------------------------------------------------------------+
// |                               Engine Dispatch Unit Status Transition Table                               |
// +---------------------+------------------------------------------------------------------------------------+
// |          C          |   Creating                                                                         |
// +---------------------+------------------------------------------------------------------------------------+
// |          R          |   Running                                                                          |
// +---------------------+------------------------------------------------------------------------------------+
// |          W          |   Waiting                                                                          |
// +---------------------+------------------------------------------------------------------------------------+
// |          I          |   Idle                                                                             |
// +---------------------+------------------------------------------------------------------------------------+
// |          D          |   Destroy                                                                          |
// +---------------------+------------------------------------------------------------------------------------+
// |          c          |   Create New Engine Dispatch Unit                                                  |
// +---------------------+------------------------------------------------------------------------------------+
// |          a          |   Activate Engine Dispatch Unit                                                    |
// +---------------------+------------------------------------------------------------------------------------+
// |          d          |   Destroy Engine Dispatch Unit                                                     |
// +---------------------+------------------------------------------------------------------------------------+
// |          w          |   Wait Engine Dispatch Unit                                                        |
// +---------------------+------------------------------------------------------------------------------------+
// |          t          |   Deactivate Engine Dispatch Unit                                                  |
// +---------------------+------------------------------------------------------------------------------------+
//
//                    ==== Engine Dispatch Unit State Machine Graph ====
// +-----------------------------------------------------------------------------------------+ from
// |   | C | R | W | I | D |                                                                 |
// +---+---+---+---+---+---+-----------------------------------------------------------------+
// | C | c |   |   |   |   |                                                                 |
// +---+---+---+---+---+---+-----------------------------------------------------------------+
// | R | a |   | a | a |   | Creating/Idle/Waiting could move to Running                     |
// +---+---+---+---+---+---+-----------------------------------------------------------------+
// | W |   | W |   |   |   | Running move to Waiting                                         |
// +---+---+---+---+---+---+-----------------------------------------------------------------+
// | I | t |   | t |   |   | Creating/Waiting move to Idle                                   |
// +---+---+---+---+---+---+-----------------------------------------------------------------+
// | D | d |   | d | d |   | Creating/Waiting/Idle could be destroyed                        |
// +-----------------------------------------------------------------------------------------+
// to

class ProcessModelEngineDispatchUnitManager {
private:
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*> _runQueue;
    std::map<ENGINE_DISPATCH_UNIT_ID, ProcessModelEngineDispatchUnitControlBlock*> _idleQueue;
    std::map<unsigned int, ENGINE_DISPATCH_UNIT_ID> _tid_engine_dispatch_unit_map;
    OSServiceSLatch _mutex;
    ENGINE_DISPATCH_UNIT_ID _ENGINE_DISPATCH_UNIT_ID;
    std::map<unsigned int, ENGINE_DISPATCH_UNIT_ID> _mapSystemEngineDispatchUnits;
    bool _isQuiesced;
    bool _isDestroyed;
public:
    ProcessModelEngineDispatchUnitManager(): _ENGINE_DISPATCH_UNIT_ID(1), _isQuiesced(false), _isDestroyed(false) {}
    ~ProcessModelEngineDispatchUnitManager();
    void reset();
    unsigned int size();
    unsigned int sizeRun();
    unsigned int sizeIdle();
    unsigned int sizeSystem();
    ENGINE_DISPATCH_UNIT_ID getSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPES type);
    bool isSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id);
    void regSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPES type, ENGINE_DISPATCH_UNIT_ID id);
    bool isQuiesced();
    void setQuiesced(bool b);
    bool isDestroyed();
    static bool isPoolable(ENGINE_DISPATCH_UNIT_TYPES type) {
        return ENGINE_DISPATCH_UNIT_TYPE_AGENT == type;
    }

    int returnEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id, bool force, bool* destroyed);
    int activateEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id);
    int waitEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id);
    int startEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPES type, void* arg, ENGINE_DISPATCH_UNIT_ID *id);
    int postEngineDispatchUnitPost(ENGINE_DISPATCH_UNIT_ID id, ProcessModelEngineDispatchUnitEventTypes type,
                                   bool release = false, void* pData = NULL);
    int waitEngineDispatchUnitPost(ENGINE_DISPATCH_UNIT_ID id, ProcessModelEngineDispatchUnitEvent& event,
                                   long long millsecs);

    ProcessModelEngineDispatchUnitControlBlock* getEngineDispatchUnit(unsigned int tid);
    ProcessModelEngineDispatchUnitControlBlock* getEngineDispatchUnit();
    ProcessModelEngineDispatchUnitControlBlock* getEngineDispatchUnitByID(ENGINE_DISPATCH_UNIT_ID eduID);
    void setEngineDispatchUnit(unsigned int tid, ENGINE_DISPATCH_UNIT_ID id);
    int forceUserEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID eduID);

private:
    int _createNewEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPES type, void* arg, ENGINE_DISPATCH_UNIT_ID* id);
    int _destroyAll();
    int _forceEngineDispatchUnits(int property = ENGINE_DISPATCH_UNIT_ALL);
    unsigned int _getEngineDispatchUnitCount(int property = ENGINE_DISPATCH_UNIT_ALL);
    void _setDestroyed(bool b);
    bool _isSystemEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id);
    int _destroyEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id);
    int _deactivateEngineDispatchUnit(ENGINE_DISPATCH_UNIT_ID id);

};

#endif
