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

//#include "process_model_agent.hpp"

#include "problem_detect.hpp"
#include "process_model_engine_dispatch_unit_manager.hpp"
#include "process_model_engine_dispatch_unit.hpp"
#include "os_service_socket.hpp"
#include "bson.h"
#include "process_model.hpp"
#include "message.hpp"

using namespace bson;
using namespace std;

#define OSServiceRoundUpToMultipleX(x,y)             (((x)+((y)-1))-(((x)+((y)-1))%(y)))
#define PROCESS_MODEL_AGENT_RECEIVE_BUFFER_SIZE      4096
#define DB_PAGE_SIZE                                 4096

static int processModelProcessAgentRequest(char *pReceiveBuffer,int packetSize, char **ppResultBuffer,
                                    int *pResultBufferSize, bool *disconnect,
                                    ProcessModelEngineDispatchUnitControlBlock *cb ){

    DB_ASSERT ( disconnect, "disconnect can't be NULL" )
    DB_ASSERT ( pReceiveBuffer, "pReceivedBuffer is NULL" )

    int rc = DB_OK ;
    unsigned int probe = 0 ;
    const char *pInsertorBuffer = NULL ;

    BSONObj recordID;
    BSONObj retObj;

    MessageHeader *header = (MessageHeader *)pReceiveBuffer ;
    int messageLength = header->messageLen ;
    int opCode = header->opCode ;
    DBKernelControlBlock *krcb = processModelGetKernelControlBlock() ;

    Runtime *rtnMgr = krcb->getRuntimeManager() ;
    *disconnect = false ;

    if ( messageLength < (int)sizeof(MessageHeader) ) {
        probe = 10 ;
        rc = DB_INVALID_ARG ;
        goto error ;
    }
    try {
        if ( OP_INSERT == opCode ) {
            int recordNum = 0 ;
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_DEBUG, "Insert request received" ) ;
            rc = msgExtractInsert (pReceiveBuffer, recordNum,&pInsertorBuffer ) ;
            if ( rc ) {
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR,
                         "Failed to read insert packet" ) ;
                probe = 20 ;
                rc = DB_INVALID_ARG ;
                goto error ;
            }
            try {
                BSONObj insertor ( pInsertorBuffer )  ;

                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_EVENT,"Insert: insertor: %s",insertor.toString().c_str() ) ;

                BSONObjIterator it ( insertor ) ;
                BSONElement ele = *it ;
                const char *tmp = ele.fieldName () ;

                rc = strcmp ( tmp, gKeyFieldName ) ;
                if ( rc ) {
                    PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "First element in inserted record is not _id" ) ;
                    probe = 25 ;
                    rc = DB_NO_ID ;
                    goto error ;
                }
                rc = rtnMgr->runtimeInsert( insertor ) ;
            } catch ( std::exception &e )
            {
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR,
                         "Failed to create insertor for insert: %s",
                         e.what() ) ;
                probe = 30 ;
                rc = DB_INVALID_ARG ;
                goto error ;
            }
        } else if ( OP_QUERY == opCode ) {
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_DEBUG, "Query request received" ) ;

            rc = msgExtractQuery ( pReceiveBuffer, recordID ) ;

            if ( rc ) {
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to read query packet" ) ;
                probe = 40 ;
                rc = DB_INVALID_ARG ;
                goto error ;
            }

            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_EVENT, "Query condition: %s", recordID.toString().c_str() ) ;
            rc = rtnMgr->runtimeFind( recordID, retObj ) ;
        }
        else if ( OP_DELETE == opCode ) {
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_DEBUG, "Delete request received" ) ;
            rc = msgExtractDelete ( pReceiveBuffer, recordID ) ;
            if ( rc ) {
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to read delete packet" ) ;
                probe = 50 ;
                rc = DB_INVALID_ARG ;
                goto error ;
            }
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_EVENT, "Delete condition: %s", recordID.toString().c_str() ) ;
            rc = rtnMgr->runtimeRemove( recordID ) ;
        } else if ( OP_SNAPSHOT == opCode ) {
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_DEBUG,
                     "Snapshot request received" ) ;
            try {
                BSONObjBuilder b ;
                b.append ( "insertTimes", 100 ) ;
                b.append ( "delTimes", 1000 ) ;
                b.append ( "queryTimes", 2000 ) ;
                b.append ( "serverRunTime", 100 ) ;
                retObj = b.obj () ;
            } catch ( std::exception &e ) {
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR,
                         "Failed to create return BSONObj: %s",
                         e.what() ) ;
                probe = 55 ;
                rc = DB_INVALID_ARG ;
                goto error ;
            }
        } else if ( OP_COMMAND == opCode ) {
        } else if ( OP_DISCONNECT == opCode ) {
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_EVENT, "Receive disconnect msg" ) ;
            *disconnect = true ;
        } else {
            probe = 60 ;
            rc = DB_INVALID_ARG ;
            goto error ;
        }
    } catch ( std::exception &e ) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Error happened during performing operation: %s", e.what() ) ;
        probe = 70 ;
        rc = DB_INVALID_ARG ;
        goto error ;
    }

    if ( rc ) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to perform operation, rc = %d", rc ) ;
        goto error ;
    }

    done :

        if ( !*disconnect ) {
            switch ( opCode ) {
                case OP_SNAPSHOT :
                case OP_QUERY :
                    msgBuildReply(ppResultBuffer, pResultBufferSize, rc, &retObj ) ;
                    break ;
                default :
                    msgBuildReply (ppResultBuffer,pResultBufferSize,rc, NULL);
                    break ;
            }
        }
        return rc ;
    error :
        switch ( rc ) {
            case DB_INVALID_ARG :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Invalid argument is received, probe: %d", probe ) ;
                break ;
            case DB_INDEX_MANAGEMENT_ID_NOT_EXIST :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Record does not exist" ) ;
                break ;
            default :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "System error, probe: %d, rc = %d", probe, rc ) ;
                break ;
        }
        goto done ;
}

int processModelAgentEntryPoint ( ProcessModelEngineDispatchUnitControlBlock *cb, void *arg ) {
    int rc = DB_OK ;
    unsigned int probe = 0 ;
    bool disconnect = false ;
    char *pReceiveBuffer = NULL ;
    char *pResultBuffer = NULL ;
    int receiveBufferSize = OSServiceRoundUpToMultipleX (PROCESS_MODEL_AGENT_RECEIVE_BUFFER_SIZE, DB_PAGE_SIZE ) ;

    int resultBufferSize  = sizeof( MessageReply ) ;
    int packetLength  = 0 ;
    ENGINE_DISPATCH_UNIT_ID myENGINE_DISPATCH_UNIT_ID = cb->getID () ;
    ProcessModelEngineDispatchUnitManager *eduMgr = cb->getEngineDispatchManager() ;

    int s = *(( int *) &arg ) ;
    OSServiceSocket sock ( &s ) ;
    sock.disableNagle () ;

    pReceiveBuffer = (char*)malloc( sizeof(char) * receiveBufferSize ) ;
    if ( !pReceiveBuffer ) {
        rc = DB_OOM ;
        probe = 10 ;
        goto error ;
    }

    pResultBuffer = (char*)malloc( sizeof(char) *
                                   resultBufferSize ) ;
    if ( !pResultBuffer ) {
        rc = DB_OOM ;
        probe = 20 ;
        goto error ;
    }

    while ( !disconnect ) {
        rc = processModelRecv( pReceiveBuffer, sizeof (int), &sock, cb ) ;
        if ( rc )
        {
            if ( DB_APP_FORCED == rc )
            {
                disconnect = true ;
                continue ;
            }
            probe = 30 ;
            goto error ;
        }
        packetLength = *(int*)(pReceiveBuffer) ;
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_DEBUG,"Received packet size = %d", packetLength ) ;

        if ( packetLength < (int)sizeof (int) ){
            probe = 40 ;
            rc = DB_INVALID_ARG ;
            goto error ;
        }

        if ( receiveBufferSize < packetLength+1 ){
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_DEBUG,"Receive buffer size is too small: %d vs %d, increasing...",
                                 receiveBufferSize, packetLength ) ;
            int newSize = OSServiceRoundUpToMultipleX (packetLength + 1, DB_PAGE_SIZE ) ;

            if ( newSize < 0 ){
                probe = 50 ;
                rc = DB_INVALID_ARG ;
                goto error ;
            }
            free ( pReceiveBuffer ) ;
            pReceiveBuffer = (char*)malloc ( sizeof(char) * (newSize) ) ;
            if ( !pReceiveBuffer ){
                rc = DB_OOM ;
                probe = 60 ;
                goto error ;
            }

            *(int*)(pReceiveBuffer) = packetLength ;
            receiveBufferSize = newSize ;
        }
        rc = processModelRecv(&pReceiveBuffer[sizeof(int)], packetLength - sizeof(int), &sock, cb) ;
        if ( rc ) {
            if ( DB_APP_FORCED == rc ) {
                disconnect = true ;
                continue ;
            }
            probe = 70 ;
            goto error ;
        }

        pReceiveBuffer[packetLength] = 0 ;

        if ( DB_OK != ( rc = eduMgr->activateEngineDispatchUnit( myENGINE_DISPATCH_UNIT_ID )) ) {
            goto error ;
        }

        if( resultBufferSize >(int)sizeof( MessageReply ) ) {
            resultBufferSize =  (int)sizeof( MessageReply ) ;
            free ( pResultBuffer ) ;

            pResultBuffer = (char*)malloc( sizeof(char) * resultBufferSize ) ;

            if ( !pResultBuffer ){
                rc = DB_OOM ;
                probe = 20 ;
                goto error ;
            }
        }
        rc = processModelProcessAgentRequest ( pReceiveBuffer,packetLength,&pResultBuffer,
                                      &resultBufferSize,&disconnect, cb) ;
        if ( rc ){
            PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Error processing Agent request, rc=%d", rc ) ;
        }

        if ( !disconnect ) {
            rc = processModelSend( pResultBuffer, *(int*)pResultBuffer, &sock, cb ) ;
            if ( rc ){
                probe = 80 ;
                goto error ;
            }
        }

        if ( DB_OK != ( rc = eduMgr->waitEngineDispatchUnit( myENGINE_DISPATCH_UNIT_ID )) ){
            goto error ;
        }
    }
    done :
        if ( pReceiveBuffer ) {
            free ( pReceiveBuffer )  ;
        }

        if ( pResultBuffer ) {
            free ( pResultBuffer )  ;
        }

        sock.close () ;
        return rc;
    error :
        switch ( rc ){
            case DB_SYS :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_SEVERE,
                         "EDU id %d cannot be found, probe %d", myENGINE_DISPATCH_UNIT_ID, probe ) ;
                break ;
            case DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_SEVERE, "EDU status is not valid, probe %d", probe ) ;
                break ;
            case DB_INVALID_ARG :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_SEVERE, "Invalid argument receieved by agent, probe %d", probe ) ;
                break ;
            case DB_OOM :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_SEVERE, "Failed to allocate memory by agent, probe %d", probe ) ;
                break ;
            case DB_NETWORK :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_SEVERE, "Network error occured, probe %d", probe ) ;
                break ;
            case DB_NETWORK_CLOSE :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_DEBUG, "Remote connection closed" ) ;
                rc = DB_OK ;
                break ;
            default :
                PROBLEM_DETECT_LOG ( PROBLEM_DETECT_SEVERE, "Internal error, probe %d", probe ) ;
        }

        goto done ;
}
