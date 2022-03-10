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

#include "process_model_main.hpp"

static int processModelResolveArguments ( int argc, char **argv ) {
    int rc = DB_OK ;

    ProcessModelOptions options ;
    rc = options.init ( argc, argv ) ;
    if ( rc ) {
        if ( DB_PROCESS_MODEL_HELP_ONLY != rc )
            PROBLEM_DETECT_LOG( PROBLEM_DETECT_ERROR, "Failed to init options, rc = %d", rc ) ;
        goto error ;
    }
    rc = processModelGetKernelControlBlock()->init ( &options ) ;
    if( rc ) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to init krcb, rc = %d", rc ) ;
        goto error ;
    }

    done :
        return rc ;
    error :
        goto done ;
}

#ifndef _WINDOWS
    static void processModelSignalHandler ( int sigNum ) {
        if ( sigNum > 0 && sigNum <= PROCESS_MODEL_MAX_SIGNALS ) {
            if ( signalHandleMap[sigNum].handle ) {
                SHUTDOWN_DB ;
            }
        }
    }
#else
    static BOOL WINAPI processModelSignalHandler(DWORD dwCtrlType) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Shutdown Database" ) ;
        EDB_SHUTDOWN_DB ;
        return TRUE;
    }
#endif

#ifndef _WINDOWS
    static int processModelSetupSignalHandler () {
        int rc = DB_OK ;
        struct sigaction newact ;
        memset ( &newact, 0, sizeof(newact) ) ;
        sigemptyset ( &newact.sa_mask ) ;

        newact.sa_flags = 0 ;
        newact.sa_handler = (__sighandler_t ) processModelSignalHandler ;

        for ( int i = 0; i < PROCESS_MODEL_MAX_SIGNALS; ++i ) {
            sigaction ( i+1, &newact, NULL ) ;
        }
        return rc ;
    }
#endif


int processModelMasterThreadMain (int argc, char **argv ) {
    int rc = DB_OK ;
    DBKernelControlBlock *krcb = processModelGetKernelControlBlock () ;
    ProcessModelEngineDispatchUnitManager *eduMgr   = krcb->getEngineDispatchUnitManager() ;
    ENGINE_DISPATCH_UNIT_ID     agentEDU = PROCESS_MODEL_INVALID_ENGINE_DISPATCH_UNIT_ID ;
#ifndef _WINDOWS
    rc = processModelSetupSignalHandler () ;
    PROBLEM_DETECT_RC_CHECK( rc, PROBLEM_DETECT_ERROR, "Failed to setup signal handler, rc = %d", rc ) ;

#else
    SetConsoleCtrlHandler(&processModelSignalHandler, TRUE);
#endif
    rc = processModelResolveArguments ( argc, argv ) ;
    if ( DB_PROCESS_MODEL_HELP_ONLY == rc ) {
        goto done ;
    }

    PROBLEM_DETECT_RC_CHECK ( rc, PROBLEM_DETECT_ERROR, "Failed to resolve argument, rc = %d", rc ) ;

    rc = eduMgr->startEngineDispatchUnit( ENGINE_DISPATCH_UNIT_TYPE_TCP_LISTENER, NULL, &agentEDU ) ;

    PROBLEM_DETECT_RC_CHECK ( rc, PROBLEM_DETECT_ERROR, "Failed to start tcp listener edu, rc = %d", rc ) ;

    while ( IS_DB_UP ) {
        sleep(1) ;
    }

    eduMgr->reset () ;

    done :
        return rc ;
    error :
        goto done ;
}

int main ( int argc, char **argv ) {
    return processModelMasterThreadMain(argc, argv);
}
