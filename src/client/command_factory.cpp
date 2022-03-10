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

#include "command_factory.hpp"

CommandFactory::CommandFactory() {
	addCommand();
}

// 用户输入一个命令生成一个ICommand
ICommand* CommandFactory::getCommandProcessor(const char* pCmd) {
	ICommand* pProcessor = NULL;

    COMMAND_MAP::iterator iter;
    iter = _cmdMap.find(pCmd);
    if (iter != _cmdMap.end()) {
        pProcessor = iter->second;
    }

    return pProcessor;
}

//CommandFactory::~CommandFactory() = default;