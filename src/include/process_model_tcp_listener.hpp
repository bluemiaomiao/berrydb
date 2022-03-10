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

#ifndef _PROCESS_MODEL_TCP_LISTENER_HPP__
#define _PROCESS_MODEL_TCP_LISTENER_HPP__


#include "core.hpp"
#include "os_service_socket.hpp"
#include "process_model_engine_dispatch_unit.hpp"
#include "process_model.hpp"
#include "process_model_engine_dispatch_unit_manager.hpp"
#include "problem_detect.hpp"

#define PROCESS_MODEL_TCP_LISTENER_RETRY 5
#define OS_SERVICE_MAX_SERVICE_NAME NI_MAXSERV

int processModelTcpListenerEntryPoint(ProcessModelEngineDispatchUnitControlBlock* cb, void* arg);

#endif