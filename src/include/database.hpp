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

#ifndef _DATABASE_HPP__
#define _DATABASE_HPP__

#include "core.hpp"
#include "os_service_socket.hpp"
#include "command_factory.hpp"

const int CMD_BUFFER_SIZE = 512;

class DB {
public:
    DB() {}
    ~DB() {}
	void start();
protected:
	void prompt();
private:
	void split(const std::string &text, char delim, std::vector<std::string> &result);
	char* readLine(char *p, int length);
	int readInput(const char *pPrompt, int numIndent);
	OSServiceSocket _sock;
	CommandFactory _cmdFactory;
	char _cmdBuffer[CMD_BUFFER_SIZE];
};


#endif
