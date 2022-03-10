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

#include "process_model_tcp_listener.hpp"

//#include "core.hpp"
//#include "process_model.hpp"
//#include "problem_detect.hpp"
//#include "os_service_socket.hpp"
//#include "process_model_engine_dispatch_unit.hpp"
//#include "process_model_engine_dispatch_unit_manager.hpp"
//
//#define PROCESS_MODEL_TCP_LISTENER_RETRY 5
//#define OS_SERVICE_MAX_SERVICE_NAME NI_MAXSERV

int processModelTcpListenerEntryPoint(ProcessModelEngineDispatchUnitControlBlock* cb, void* arg) {
    int rc = DB_OK;

    ProcessModelEngineDispatchUnitManager *eduMgr = cb->getEngineDispatchManager();

    ENGINE_DISPATCH_UNIT_ID myEngineDispatchUnitID = cb->getID();

    unsigned int retry = 0;

    ENGINE_DISPATCH_UNIT_ID agentEDU = PROCESS_MODEL_INVALID_ENGINE_DISPATCH_UNIT_ID;

    char svcName[OS_SERVICE_MAX_SERVICE_NAME + 1];

    while (retry <= PROCESS_MODEL_TCP_LISTENER_RETRY && !IS_DB_DOWN) {
        retry++;

        strcpy(svcName, processModelGetKernelControlBlock()->getServiceName());
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_EVENT, "Listening on port_test %s\n", svcName);

        int port = 0;
        int len = strlen(svcName);
        for (int i = 0; i < len; ++i) {
            if (svcName[i] >= '0' && svcName[i] <= '9') {
                port = port * 10;
                port += int(svcName[i] - '0');
            } else {
                PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "service name error!\n");
            }
        }

        OSServiceSocket sock(port);
        rc = sock.initSocket();
        DB_VALIDATE_GOTO_ERROR (DB_OK == rc, rc, "Failed initialize socket");

        rc = sock.bindListen();
        DB_VALIDATE_GOTO_ERROR (DB_OK == rc, rc, "Failed to bind/listen socket");

        if (DB_OK != (rc = eduMgr->activateEngineDispatchUnit(myEngineDispatchUnitID))) {
            goto error;
        }
        while (!IS_DB_DOWN) {
            int s;
            rc = sock.accept((SOCKET *) &s, NULL, NULL);

            if (DB_TIMEOUT == rc) {
                rc = DB_OK;
                continue;
            }

            if (rc && IS_DB_DOWN) {
                rc = DB_OK;
                goto done;
            } else if (rc) {

                PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to accept socket in TcpListener");
                PROBLEM_DETECT_LOG (PROBLEM_DETECT_EVENT, "Restarting socket to listen");
                break;
            }

            void *pData = NULL;
            *((int *) &pData) = s;

            rc = eduMgr->startEngineDispatchUnit(ENGINE_DISPATCH_UNIT_TYPE_AGENT, pData, &agentEDU);
            if (rc) {
                if (rc == DB_QUIESCED) {

                    PROBLEM_DETECT_LOG (PROBLEM_DETECT_WARNING, "Reject new connection due to quiesced database");
                } else {
                    PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to start EDU agent");
                }

                OSServiceSocket newsock((SOCKET *) &s);
                newsock.close();
                continue;
            }
        }
        if (DB_OK != (rc = eduMgr->waitEngineDispatchUnit(myEngineDispatchUnitID))) {
            goto error;
        }
    }

    done :
        return rc;
    error :
        if (rc == DB_SYS) {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_SEVERE, "System error occurred");
        } else {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_SEVERE, "Internal error");
        }

        goto done;
}
