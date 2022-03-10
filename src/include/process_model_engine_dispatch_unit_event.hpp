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

#ifndef _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_HPP__
#define _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_HPP__

#include "core.hpp"

enum ProcessModelEngineDispatchUnitEventTypes {
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_NONE = 0,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_TERM,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_RESUME,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_ACTIVE,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_DEACTIVE,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_MSG,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_TIMEOUT,
    PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_LOCK_WAKEUP
};

class ProcessModelEngineDispatchUnitEvent {
public:
    ProcessModelEngineDispatchUnitEventTypes _eventType;
    bool _release;
    void* Data;
    ProcessModelEngineDispatchUnitEvent();
    ProcessModelEngineDispatchUnitEvent(ProcessModelEngineDispatchUnitEventTypes type);
    ProcessModelEngineDispatchUnitEvent(ProcessModelEngineDispatchUnitEventTypes type, bool release, void* data);
    void reset();
};

#endif

