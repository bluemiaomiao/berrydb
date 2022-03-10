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

#ifndef _COMMAND_FACTORY_HPP__
#define _COMMAND_FACTORY_HPP__

#include "command.hpp"

#define COMMAND_BEGIN void CommandFactory::addCommand() {
#define COMMAND_END }
#define COMMAND_ADD(cmdName, cmdClass) {                \
	ICommand* pObj = new cmdClass();                    \
	std::string str = cmdName;                          \
	_cmdMap.insert(COMMAND_MAP::value_type(str, pObj)); \
}                                                       \

class CommandFactory {
	typedef std::map<std::string, ICommand*> COMMAND_MAP;
public:
	CommandFactory();
	~CommandFactory() {}
	void addCommand();
	ICommand* getCommandProcessor(const char* pCmd);
private:
	COMMAND_MAP _cmdMap;
};

#endif
