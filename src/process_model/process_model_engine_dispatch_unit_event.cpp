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

#include "process_model_engine_dispatch_unit_event.hpp"

ProcessModelEngineDispatchUnitEvent::ProcessModelEngineDispatchUnitEvent() {
    this->_eventType = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_NONE;
    this->_release = false;
    this->Data = NULL;
}

ProcessModelEngineDispatchUnitEvent::ProcessModelEngineDispatchUnitEvent( ProcessModelEngineDispatchUnitEventTypes type) {
    this->_eventType = type;
    this->_release = false;
    this->Data = NULL;
}

ProcessModelEngineDispatchUnitEvent::ProcessModelEngineDispatchUnitEvent(ProcessModelEngineDispatchUnitEventTypes type, bool release, void* data) {
    this->_eventType = type;
    this->_release = release;
    this->Data = data;
}

void ProcessModelEngineDispatchUnitEvent::reset() {
    this->_eventType = PROCESS_MODEL_ENGINE_DISPATCH_UNIT_EVENT_NONE;
    this->_release = false;
    this->Data = NULL;

}
