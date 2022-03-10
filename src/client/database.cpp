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

#include <iostream>
#include <sstream>
#include "core.hpp"
#include "database.hpp"
#include "command.hpp"

const char SPACE = ' ';
const char TAB = '\t';
const char BACK_SLANT = '\\';
const char NEW_LINE = '\n';

int gQuit = 0;

void DB::start() {
	printf("Welcome to BerryDB Shell.\n");
	printf("db help for help, Ctrl(Command)+C or quit to exit\n");
	
	while(gQuit == 0) { prompt(); }

	return;
}

void DB::prompt() {
	int rc = DB_OK;
	
	rc = readInput("db", 0);
	if(rc) { return; }

	// 将输入的字符串打成段并保存到Vector中
	std::string textInput = _cmdBuffer;
	std::vector<std::string> textVec;
	split(textInput, SPACE, textVec);
	int cnt = 0;
	std::string cmd = "";
	std::vector<std::string> optVec;
	std::vector<std::string>::iterator iter = textVec.begin();

	ICommand* pCmd = NULL;

	for(;iter != textVec.end(); ++iter) {
		std::string str = *iter;
		if(cnt == 0) {
			cmd = str;
			cnt++;
		} else {
			optVec.push_back(str);
		}
	}
	
	// 将Vector中的命令转为一个可执行的命令
	pCmd = _cmdFactory.getCommandProcessor(cmd.c_str());
	if(pCmd != NULL) {
		pCmd->execute(_sock, optVec);
	}

	return;
}

int DB::readInput(const char* pPrompt, int numIndent) {
	memset(_cmdBuffer, 0, CMD_BUFFER_SIZE);

	for(int i = 0; i < numIndent; i++) { 
		std::cout << TAB;
	}
	
	std::cout << pPrompt << "> ";
	
	readLine(_cmdBuffer, CMD_BUFFER_SIZE - 1);

	int curBufLen = strlen(_cmdBuffer);
	while(_cmdBuffer[curBufLen - 1] == BACK_SLANT && (CMD_BUFFER_SIZE - curBufLen) > 0) {
		for(int i = 0; i < numIndent; i++) { 
			std::cout << TAB; 
		}
		std:: cout << "> ";
		readLine(&_cmdBuffer[curBufLen - 1], CMD_BUFFER_SIZE - curBufLen);
	}

	curBufLen = strlen(_cmdBuffer);
	for(int i = 0; i < curBufLen; i++) {
		if(_cmdBuffer[i] == TAB) { _cmdBuffer[i] = SPACE; }
	}

	return DB_OK;
}

char* DB::readLine(char* p, int length) {
	int len = 0;
	int ch;

	while((ch = getchar()) != NEW_LINE) {
		switch(ch) {
			case BACK_SLANT:
				break;
			default:
				p[len] = ch;
				len++;
		}
	}

	len  = strlen(p);
	p[len] = 0;
	return p;
}

void DB::split(const std::string &text, char delim, std::vector<std::string> &result) {
	size_t strLen = text.length();
	size_t first = 0;
	size_t pos = 0;

	for(first = 0; first < strLen; first = pos + 1) {
		pos = first;
		while(text[pos] != delim && pos < strLen) {
			pos++;
		}
		std::string str = text.substr(first, pos - first);
		result.push_back(str);
	}

	return;
}

int main(int argc, char** argv) {
	DB db;
	db.start();
	return 0;
}
