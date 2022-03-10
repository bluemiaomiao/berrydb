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

#include "core.hpp"
#include "message.hpp"
#include "problem_detect.hpp"

using namespace bson;

static int msgCheckBuffer(char **ppBuffer, int *pBufferSize, int length) {
    int rc = DB_OK;

    if (length > *pBufferSize) {
        char *pOldBuf = *ppBuffer;

        if (length < 0) {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "invalid length: %d", length);
            rc = DB_INVALID_ARG;
            goto error;
        }

        *ppBuffer = (char *) realloc(*ppBuffer, sizeof(char) * length);

        if (!*ppBuffer) {
            PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to allocate %d bytes buffer", length);
            rc = DB_OOM;
            *ppBuffer = pOldBuf;
            goto error;
        }

        *pBufferSize = length;
    }

    done:
        return rc;
    error:
        goto done;
}

int msgBuildReply(char **ppBuffer, int *pBufferSize, int returnCode, BSONObj *objReturn) {
    int rc = DB_OK;
    int size = sizeof(MessageReply);
    MessageReply *pReply = NULL;

    if (objReturn) {
        size += objReturn->objsize();
    }

    rc = msgCheckBuffer(ppBuffer, pBufferSize, size);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to realloc buffer for %d bytes, rc = %d", size, rc);

    pReply = (MessageReply *) (*ppBuffer);

    pReply->header.messageLen = size;
    pReply->header.opCode = OP_REPLY;

    pReply->returnCode = returnCode;
    pReply->numReturn = (objReturn) ? 1 : 0;

    if (objReturn) {
        memcpy(&pReply->data[0], objReturn->objdata(), objReturn->objsize());
    }

    done:
        return rc;
    error:
        goto done;
}


int msgExtractReply(char *pBuffer, int &returnCode, int &numReturn, const char **ppObjStart) {
    int rc = DB_OK;
    MessageReply *pReply = (MessageReply *) pBuffer;

    if (pReply->header.messageLen < (int) sizeof(MessageReply)) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid length of reply message");
        rc = DB_INVALID_ARG;
        goto error;
    }

    if (pReply->header.opCode != OP_REPLY) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "non-reply code is received: %d, expected %d",
                            pReply->header.opCode, OP_REPLY);
        rc = DB_INVALID_ARG;
        goto error;
    }

    returnCode = pReply->returnCode;
    numReturn = pReply->numReturn;

    if (0 == numReturn) {
        *ppObjStart = NULL;
    } else {
        *ppObjStart = &pReply->data[0];
    }

    done:
        return rc;
    error:
        goto done;
}

int msgBuildInsert(char **ppBuffer, int *pBufferSize, BSONObj &obj) {
    int rc = DB_OK;
    int size = sizeof(MessageInsert) + obj.objsize();

    MessageInsert *pInsert = NULL;

    rc = msgCheckBuffer(ppBuffer, pBufferSize, size);

    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to realloc buffer for %d bytes, rc = %d", size, rc);
        goto error;
    }

    pInsert = (MessageInsert *) (*ppBuffer);

    pInsert->header.messageLen = size;
    pInsert->header.opCode = OP_INSERT;

    pInsert->numInsert = 1;

    memcpy(&pInsert->data[0], obj.objdata(), obj.objsize());

    done:
        return rc;
    error:
        goto done;
}

int msgBuildInsert(char **ppBuffer, int *pBufferSize, vector<BSONObj *> &obj) {
    int rc = DB_OK;

    int size = sizeof(MessageInsert);

    MessageInsert *pInsert = NULL;

    vector<BSONObj *>::iterator it;

    char *p = NULL;

    for (it = obj.begin(); it != obj.end(); ++it) {
        size += (*it)->objsize();
    }

    rc = msgCheckBuffer(ppBuffer, pBufferSize, size);

    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to realloc buffer for %d bytes, rc = %d", size, rc);
        goto error;
    }

    pInsert = (MessageInsert *) (*ppBuffer);

    pInsert->header.messageLen = size;
    pInsert->header.opCode = OP_INSERT;

    pInsert->numInsert = obj.size();

    p = &pInsert->data[0];
    for (it = obj.begin(); it != obj.end(); ++it) {
        memcpy(p, (*it)->objdata(), (*it)->objsize());
        p += (*it)->objsize();
    }

    done:
        return rc;
    error:
        goto done;
}

int msgExtractInsert(char *pBuffer, int &numInsert, const char **ppObjStart) {
    int rc = DB_OK;

    MessageInsert *pInsert = (MessageInsert *) pBuffer;

    if (pInsert->header.messageLen < (int) sizeof(MessageInsert)) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid length of insert message");
        rc = DB_INVALID_ARG;
        goto error;
    }

    if (pInsert->header.opCode != OP_INSERT) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "non-insert code is received: %d, expected %d",
                            pInsert->header.opCode, OP_INSERT);
        rc = DB_INVALID_ARG;
        goto error;
    }

    numInsert = pInsert->numInsert;

    if (0 == numInsert) {
        *ppObjStart = NULL;
    } else {
        *ppObjStart = &pInsert->data[0];
    }

    done:
        return rc;
    error:
        goto done;
}

int msgBuildDelete(char **ppBuffer, int *pBufferSize, BSONObj &key) {
    int rc = DB_OK;

    int size = sizeof(MessageDelete) + key.objsize();

    MessageDelete *pDelete = NULL;

    rc = msgCheckBuffer(ppBuffer, pBufferSize, size);
    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to realloc buffer for %d bytes, rc = %d", size, rc);
        goto error;
    }

    pDelete = (MessageDelete *) (*ppBuffer);

    pDelete->header.messageLen = size;
    pDelete->header.opCode = OP_DELETE;

    memcpy(&pDelete->key[0], key.objdata(), key.objsize());

    done:
        return rc;
    error:
        goto done;
}

int msgExtractDelete(char *pBuffer, BSONObj &key) {
    int rc = DB_OK;

    MessageDelete *pDelete = (MessageDelete *) pBuffer;

    if (pDelete->header.messageLen < (int) sizeof(MessageDelete)) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid length of delete message");
        rc = DB_INVALID_ARG;
        goto error;
    }

    if (pDelete->header.opCode != OP_DELETE) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "non-delete code is received: %d, expected %d",
                            pDelete->header.opCode, OP_DELETE);
        rc = DB_INVALID_ARG;
        goto error;
    }

    key = BSONObj(&pDelete->key[0]);

    done:
        return rc;
    error:
        goto done;
}

int msgBuildQuery(char **ppBuffer, int *pBufferSize, BSONObj &key) {
    int rc = DB_OK;

    int size = sizeof(MessageQuery) + key.objsize();

    MessageQuery *pQuery = NULL;

    rc = msgCheckBuffer(ppBuffer, pBufferSize, size);
    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to realloc buffer for %d bytes, rc = %d", size, rc);
        goto error;
    }

    pQuery = (MessageQuery *) (*ppBuffer);

    pQuery->header.messageLen = size;
    pQuery->header.opCode = OP_QUERY;

    memcpy(&pQuery->key[0], key.objdata(), key.objsize());

    done:
        return rc;
    error :
        goto done;
}

int msgExtractQuery(char *pBuffer, BSONObj &key) {
    int rc = DB_OK;

    MessageQuery *pQuery = (MessageQuery *) pBuffer;

    if (pQuery->header.messageLen < (int) sizeof(MessageQuery)) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid length of query message");
        rc = DB_INVALID_ARG;
        goto error;
    }

    if (pQuery->header.opCode != OP_QUERY) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "non-query code is received: %d, expected %d",
                            pQuery->header.opCode, OP_QUERY);
        rc = DB_INVALID_ARG;
        goto error;
    }

    key = BSONObj(&pQuery->key[0]);

    done :
        return rc;
    error :
        goto done;
}

int msgBuildCommand(char **ppBuffer, int *pBufferSize, BSONObj &obj) {
    int rc = DB_OK;

    int size = sizeof(MessageCommand) + obj.objsize();

    MessageCommand *pCommand = NULL;

    rc = msgCheckBuffer(ppBuffer, pBufferSize, size);
    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to realloc buffer for %d bytes, rc = %d", size, rc);
        goto error;
    }

    pCommand = (MessageCommand *) (*ppBuffer);

    pCommand->header.messageLen = size;
    pCommand->header.opCode = OP_COMMAND;

    pCommand->numArgs = 1;

    memcpy(&pCommand->data[0], obj.objdata(), obj.objsize());

    done :
        return rc;
    error :
        goto done;
}

int msgBuildCommand(char **ppBuffer, int *pBufferSize, vector<BSONObj *> &obj) {
    int rc = DB_OK;

    int size = sizeof(MessageCommand);

    MessageCommand *pCommand = NULL;

    vector<BSONObj *>::iterator it;

    char *p = NULL;

    for (it = obj.begin(); it != obj.end(); ++it) {
        size += (*it)->objsize();
    }

    rc = msgCheckBuffer(ppBuffer, pBufferSize, size);
    if (rc) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to realloc buffer for %d bytes, rc = %d", size, rc);
        goto error;
    }

    pCommand = (MessageCommand *) (*ppBuffer);

    pCommand->header.messageLen = size;
    pCommand->header.opCode = OP_COMMAND;

    pCommand->numArgs = obj.size();

    p = &pCommand->data[0];
    for (it = obj.begin(); it != obj.end(); ++it) {
        memcpy(p, (*it)->objdata(), (*it)->objsize());
        p += (*it)->objsize();
    }

    done :
        return rc;
    error :
        goto done;
}

int msgExtractCommand(char *pBuffer, int &numArgs, const char **ppObjStart) {
    int rc = DB_OK;

    MessageCommand *pCommand = (MessageCommand *) pBuffer;

    if (pCommand->header.messageLen < (int) sizeof(MessageCommand)) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Invalid length of command message");
        rc = DB_INVALID_ARG;
        goto error;
    }

    if (pCommand->header.opCode != OP_COMMAND) {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "non-command code is received: %d, expected %d",
                            pCommand->header.opCode, OP_COMMAND);
        rc = DB_INVALID_ARG;
        goto error;
    }

    numArgs = pCommand->numArgs;

    if (0 == numArgs) {
        *ppObjStart = NULL;
    } else {
        *ppObjStart = &pCommand->data[0];
    }

    done :
        return rc;
    error :
        goto done;
}