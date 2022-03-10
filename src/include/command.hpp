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

#ifndef _COMMAND_HPP__
#define _COMMAND_HPP__

#include "core.hpp"
#include "json.h"
#include "os_service_socket.hpp"

#define COMMAND_QUIT            "quit"
#define COMMAND_INSERT          "insert"
#define COMMAND_QUERY           "query"
#define COMMAND_DELETE          "delete"
#define COMMAND_HELP            "help"
#define COMMAND_CONNECT         "connect"
#define COMMAND_TEST            "test"
#define COMMAND_SNAPSHOT        "snapshot"

#define RECV_BUFF_SIZE  4096
#define SEND_BUFF_SIZE  4096

#define DB_QUERY_INVALID_ARG   -101
#define DB_INSERT_INVALID_ARG  -102
#define DB_DELETE_INVALID_ARG  -103
#define DB_INVALID_RECORD      -104
#define DB_RECV_DATA_LEN_ERROR -107
#define DB_SOCK_INIT_FAILED    -113
#define DB_SOCK_CONNECT_FAILED -114
#define DB_SOCK_NOT_CONNECT    -115
#define DB_SOCK_REMOTE_CLOSE   -116
#define DB_SOCK_SEND_FAILED    -117
#define DB_MSG_BUILD_FAILED    -119

class ICommand {
	typedef int (*OnMsgBuild) (char** ppBuffer, int* pBufferSize, bson::BSONObj &obj);
public:
	virtual int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
	int getError(int code);
protected:
	int recvReply(OSServiceSocket &sock);
	int sendOrder(OSServiceSocket &sock, OnMsgBuild onMsgBuild);
	int sendOrder(OSServiceSocket &sock, int opCode);
	virtual int handleReply() { return DB_OK; }
	char _recvBuff[RECV_BUFF_SIZE];
	char _sendBuff[SEND_BUFF_SIZE];
	std::string _jsonString;
};

class ConnectCommand : public ICommand {
public:
	int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
private:
      std::string _address;
      int _port;
};


class QuitCommand : public ICommand {
public:
	int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
protected:
	int handleReply();
};

class HelpCommand : public ICommand {
public:
	int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
};

class SnapshotCommand : public ICommand {
public:
	int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
protected:
	int handleReply();
};

class InsertCommand : public ICommand {
public:
	int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
protected:
	int handleReply();
};

class QueryCommand : public ICommand {
public:
	int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
protected:
	int handleReply();
};

class DeleteCommand : public ICommand {
public:
int execute(OSServiceSocket &sock, std::vector<std::string> &argVec);
protected:
	int handleReply();
};

#endif
