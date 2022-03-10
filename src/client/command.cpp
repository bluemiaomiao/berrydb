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
#include "command.hpp"
#include "command_factory.hpp"
#include "problem_detect.hpp"
#include "message.hpp"

COMMAND_BEGIN
    COMMAND_ADD(COMMAND_INSERT, InsertCommand)
    COMMAND_ADD(COMMAND_QUERY, QueryCommand)
    COMMAND_ADD(COMMAND_DELETE, DeleteCommand)
    COMMAND_ADD(COMMAND_CONNECT, ConnectCommand)
    COMMAND_ADD(COMMAND_QUIT, QuitCommand)
    COMMAND_ADD(COMMAND_HELP, HelpCommand)
    COMMAND_ADD(COMMAND_SNAPSHOT, SnapshotCommand)
COMMAND_END

extern int gQuit;

int ICommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    return DB_OK;
}

int ICommand::getError(int code) {
    switch (code) {
        case DB_OK:
            break;
        case DB_IO:
            std::cout << "I/O error is occurred" << std::endl;
            break;
        case DB_INVALID_ARG:
            std::cout << "invalid argument" << std::endl;
            break;
        case DB_PERMIT:
            std::cout << "permit error" << std::endl;
            break;
        case DB_OOM:
            std::cout << "out of memory" << std::endl;
            break;
        case DB_SYS:
            std::cout << "system error is occurred" << std::endl;
            break;
        case DB_PROCESS_MODEL_HELP_ONLY:
            break;
        case DB_PROCESS_MODEL_FORCE_SYS_ENGINE_DISPATCH_UNIT:
            break;
        case DB_TIMEOUT:
            break;
        case DB_QUIESCED:
            std::cout << "quiesced" << std::endl;
            break;
        case DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS:
            std::cout << "invalid status" << std::endl;
            break;
        case DB_NETWORK:
            std::cout << "network error" << std::endl;
            break;
        case DB_NETWORK_CLOSE:
            std::cout << "network socket is closed" << std::endl;
            break;
        case DB_APP_FORCED:
            break;
        case DB_INDEX_MANAGEMENT_ID_EXIST:
            std::cout << "index management sub-system id is exist" << std::endl;
            break;
        case DB_HEADER_INVALID:
            std::cout << "record header is not ok" << std::endl;
            break;
        case DB_INDEX_MANAGEMENT_ID_NOT_EXIST:
            std::cout << "index management sub-system id is not exist" << std::endl;
            break;
        case DB_NO_ID:
            std::cout << "_id element is needed" << std::endl;
            break;
        case DB_QUERY_INVALID_ARG:
            std::cout << "invalid query argument" << std::endl;
            break;
        case DB_INSERT_INVALID_ARG:
            std::cout << "invalid insert argument" << std::endl;
            break;
        case DB_DELETE_INVALID_ARG:
            std::cout << "invalid delete argument" << std::endl;
            break;
        case DB_INVALID_RECORD:
            std::cout << "invalid record string" << std::endl;
            break;
        case DB_SOCK_NOT_CONNECT:
            std::cout << "socket connection does not exist" << std::endl;
            break;
        case DB_SOCK_REMOTE_CLOSE:
            std::cout << "remote socket connection is closed" << std::endl;
            break;
        case DB_MSG_BUILD_FAILED:
            std::cout << "message build failed" << std::endl;
            break;
        case DB_SOCK_SEND_FAILED:
            std::cout << "socket send message failed" << std::endl;
            break;
        case DB_SOCK_INIT_FAILED:
            std::cout << "socket connection initialize failed" << std::endl;
            break;
        case DB_SOCK_CONNECT_FAILED:
            std::cout << "socket connect to remote server failed" << std::endl;
            break;
        case DB_RECV_DATA_LEN_ERROR:
            std::cout << "receive data length error" << std::endl;
            break;
        default:
            std::cout << "unknown error" << std::endl;
            break;
    }

    return code;
}

int ICommand::recvReply(OSServiceSocket &sock) {
    // 定义消息长度
    int length = 0;
    int res = DB_OK;
    memset(this->_recvBuff, 0, RECV_BUFF_SIZE);

    if (!sock.isConnected()) {
        return getError(DB_SOCK_NOT_CONNECT);
    }

    while (true) {
        // 接受消息长度信息
        res = sock.recv(this->_recvBuff, sizeof(int));
        if (res == DB_TIMEOUT) {
            continue;
        } else if (res == DB_NETWORK_CLOSE) {
            return getError(DB_SOCK_REMOTE_CLOSE);
        } else {
            break;
        }
    }

    // 解析出长度信息
    length = *(int *) this->_recvBuff;
    if (length > RECV_BUFF_SIZE) {
        return getError(DB_RECV_DATA_LEN_ERROR);
    }

    while (true) {
        res = sock.recv(&this->_recvBuff[sizeof(int)], length - sizeof(int));
        if (res == DB_TIMEOUT) {
            continue;
        } else if (res == DB_NETWORK_CLOSE) {
            return getError(DB_SOCK_REMOTE_CLOSE);
        } else {
            break;
        }
    }

    return res;
}

int ICommand::sendOrder(OSServiceSocket &sock, OnMsgBuild onMsgBuild) {
    int res = DB_OK;
    bson::BSONObj bsonData;

    try {
        bsonData = bson::fromjson(this->_jsonString);
    } catch (std::exception &e) {
        return getError(DB_INVALID_RECORD);
    }

    memset(this->_sendBuff, 0, SEND_BUFF_SIZE);

    int size = SEND_BUFF_SIZE;
    char *pSendBuf = this->_sendBuff;

    res = onMsgBuild(&pSendBuf, &size, bsonData);
    if (res) {
        return getError(DB_MSG_BUILD_FAILED);
    }

    res = sock.send(pSendBuf, *(int *) pSendBuf);
    if (res) {
        return getError(DB_SOCK_SEND_FAILED);
    }

    return res;
}

int ICommand::sendOrder(OSServiceSocket &sock, int opCode) {
    int res = DB_OK;

    memset(this->_sendBuff, 0, SEND_BUFF_SIZE);

    char *pSendBuf = this->_sendBuff;

    MessageHeader *header = (MessageHeader *) pSendBuf;
    header->messageLen = sizeof(MessageHeader);
    header->opCode = opCode;

    res = sock.send(pSendBuf, *(int *) pSendBuf);

    return res;
}

int ConnectCommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    int rc = DB_OK;
    _address = argVec[0];
    _port = atoi(argVec[1].c_str());

    std::cout << "Connecting to " << this->_address << ":" << this->_port << " ...";

    sock.close();
    sock.setAddress(this->_address.c_str(), this->_port);

    rc = sock.initSocket();
    if (rc) {
        printf("Failed to initialize socket, rc=%d\n", rc);
        goto error;
    }

    rc = sock.connect();
    if (rc) {
        printf("Failed to connect, rc=%d\n", rc);
        goto error;
    }

    sock.disableNagle();

    if (sock.isConnected()) {
        std::cout << "    done" << std::endl;
    } else {
        std::cout << "    failed" << std::endl;
    }

    goto done;

    done:
        return rc;
    error:
        goto done;
}

int QuitCommand::handleReply() {
    int res = DB_OK;
    return res;
}

int QuitCommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    int res = DB_OK;
    if (!sock.isConnected()) {
        return getError(DB_SOCK_NOT_CONNECT);
    }

    res = sendOrder(sock, 0);
    res = handleReply();
    return res;
}

int HelpCommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    int res = DB_OK;

    printf("List of classes of commands:\n");
    printf("%s [server] [port] -- connecting berrydb instance.\n", COMMAND_CONNECT);
    printf("%s -- sending a insert command to berrydb instance.\n", COMMAND_INSERT);
    printf("%s -- sending a query command to berrydb instance.\n", COMMAND_QUERY);
    printf("%s -- sending a delete command to berrydb instance.\n", COMMAND_DELETE);
    printf("%s [number] -- sending a test command to berrydb instance.\n", COMMAND_TEST);
    printf("%s -- providing current number of record inserting.\n", COMMAND_SNAPSHOT);
    printf("%s -- quitting command.\n", COMMAND_QUIT);
    printf("Type \"help\" command for help.\n");

    return res;
}

int InsertCommand::handleReply() {
    MessageReply *msg = (MessageReply *) this->_recvBuff;

    int returnCode = msg->returnCode;
    int ret = getError(returnCode);

    return ret;
}

int InsertCommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    int rc = DB_OK;

    if (argVec.empty()) {
        return getError(DB_INSERT_INVALID_ARG);
    }

    this->_jsonString = argVec[0];

    if (!sock.isConnected()) {
        return getError(DB_SOCK_NOT_CONNECT);
    }

    rc = sendOrder(sock, msgBuildInsert);
    PROBLEM_DETECT_RC_CHECK(rc, PROBLEM_DETECT_ERROR, "Failed to send order, rc = %d", rc);

    rc = recvReply(sock);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);

    rc = handleReply();
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);

    done :
        return rc;
    error :
        goto done;
}

int QueryCommand::handleReply() {
    MessageReply *msg = (MessageReply *) this->_recvBuff;
    int returnCode = msg->returnCode;
    int ret = getError(returnCode);
    if (ret) {
        return ret;
    }

    if (msg->numReturn) {
        bson::BSONObj bsonData = bson::BSONObj(&(msg->data[0]));
        std::cout << bsonData.toString() << std::endl;
    }

    return ret;
}

int QueryCommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    int rc = DB_OK;

    if (argVec.empty()) {
        return getError(DB_QUERY_INVALID_ARG);
    }
    _jsonString = argVec[0];

    if (!sock.isConnected()) {
        return getError(DB_SOCK_NOT_CONNECT);
    }

    rc = sendOrder(sock, msgBuildQuery);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to send order, rc = %d", rc);

    rc = recvReply(sock);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);

    rc = handleReply();
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);

    done :
        return rc;
    error :
        goto done;
}

int DeleteCommand::handleReply() {
    MessageReply *msg = (MessageReply *) this->_recvBuff;
    int returnCode = msg->returnCode;
    int ret = getError(returnCode);
    return ret;
}

int DeleteCommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    int rc = DB_OK;

    if (argVec.empty()) {
        return getError(DB_DELETE_INVALID_ARG);
    }

    _jsonString = argVec[0];

    if (!sock.isConnected()) {
        return getError(DB_SOCK_NOT_CONNECT);
    }

    rc = sendOrder(sock, msgBuildDelete);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to send order, rc = %d", rc);

    rc = recvReply(sock);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);

    rc = handleReply();
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);

    done :
        return rc;
    error :
        goto done;
}

int SnapshotCommand::handleReply() {
    int ret = DB_OK;
    MessageReply *msg = (MessageReply *) this->_recvBuff;
    int returnCode = msg->returnCode;

    ret = getError(returnCode);
    if (ret) {
        return ret;
    }
    bson::BSONObj bsonData = bson::BSONObj(&(msg->data[0]));
    printf("insert times is %d\n", bsonData.getIntField("insertTimes"));
    printf("del times is %d\n", bsonData.getIntField("delTimes"));
    printf("query times is %d\n", bsonData.getIntField("queryTimes"));
    printf("server run time is %dm\n", bsonData.getIntField("serverRunTime"));

    return ret;
}

int SnapshotCommand::execute(OSServiceSocket &sock, std::vector<std::string> &argVec) {
    int rc = DB_OK;
    if (!sock.isConnected()) {
        return getError(DB_SOCK_NOT_CONNECT);
    }

    rc = sendOrder(sock, OP_SNAPSHOT);
    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to send order, rc = %d", rc);
    rc = recvReply(sock);

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);
    rc = handleReply();

    PROBLEM_DETECT_RC_CHECK (rc, PROBLEM_DETECT_ERROR, "Failed to receive reply, rc = %d", rc);

    done :
        return rc;
    error :
        goto done;
}
