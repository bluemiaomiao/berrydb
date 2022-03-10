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

#ifndef _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_OPTIONS_HPP__
#define _PROCESS_MODEL_ENGINE_DISPATCH_UNIT_OPTIONS_HPP__

#include "core.hpp"
#include <boost/program_options.hpp>
#include <boost/program_options/parsers.hpp>

using namespace std;

namespace po = boost::program_options;

#define PROCESS_MODEL_OPTION_HELP                           "help"
#define PROCESS_MODEL_OPTION_DBPATH                         "dbpath"
#define PROCESS_MODEL_OPTION_SVCNAME                        "svcname"
#define PROCESS_MODEL_OPTION_MAXPOOL                        "maxpool"
#define PROCESS_MODEL_OPTION_LOGPATH                        "logpath"
#define PROCESS_MODEL_OPTION_CONFPATH                       "confpath"

#define PROCESS_MODEL_ADD_PARAM_OPTIONS_BEGIN(desc)\
        desc.add_options()
#define PROCESS_MODEL_ADD_PARAM_OPTIONS_END

#define PROCESS_MODEL_COMMANDS_STRING(a, b)(string(a) + string(b)).c_str()

#define PROCESS_MODEL_COMMANDS_OPTIONS \
        ( PROCESS_MODEL_COMMANDS_STRING ( PROCESS_MODEL_OPTION_HELP, ",h"), "help" )               \
        ( PROCESS_MODEL_COMMANDS_STRING ( PROCESS_MODEL_OPTION_DBPATH, ",d"),                      \
        boost::program_options::value<string>(), "Instance database file full path." )             \
        ( PROCESS_MODEL_COMMANDS_STRING ( PROCESS_MODEL_OPTION_SVCNAME, ",s"),                     \
        boost::program_options::value<string>(), "Instance local service name." )                  \
        ( PROCESS_MODEL_COMMANDS_STRING ( PROCESS_MODEL_OPTION_MAXPOOL, ",m"),                     \
        boost::program_options::value<unsigned int>(), "Instance max pooled agent." )              \
        ( PROCESS_MODEL_COMMANDS_STRING ( PROCESS_MODEL_OPTION_LOGPATH, ",l"),                     \
        boost::program_options::value<string>(), "Instance diagnostic log file full path." )       \
        ( PROCESS_MODEL_COMMANDS_STRING ( PROCESS_MODEL_OPTION_CONFPATH, ",c"),                    \
        boost::program_options::value<string>(), "Instance configuration file full path." )        \

#define CONFIG_FILENAME                                     "berrydb.conf"
#define LOG_FILENAME                                        "server.log"
#define DB_FILENAME                                         "berry.db"
#define SVC_NAME                                            "2022"
#define NUM_POOL                                            20

class ProcessModelOptions {
public:
    ProcessModelOptions();
    ~ProcessModelOptions();
    int readCmd(int argc, char** argv, po::options_description &desc, po::variables_map &vm);
    int importVM(const po::variables_map &vm, bool isDefault = true);
    int readConfigureFile(const char* pPath, po::options_description &desc, po::variables_map &vm);
    int init(int argc, char** argv);
    inline char* getDBPath() { return this->_dbPath; }
    inline char* getLogPath() { return this->_logPath; }
    inline char* getConfPath() { return this->_confPath; }
    inline char* getServiceName() { return this->_svcName; }
    inline int getMaxPool() { return this->_maxPool; }

private:
    char _dbPath[OS_SERVICE_MAX_PATHSIZE + 1];
    char _logPath[OS_SERVICE_MAX_PATHSIZE + 1];
    char _confPath[OS_SERVICE_MAX_PATHSIZE + 1];
    char _svcName[NI_MAXSERV + 1];
    int _maxPool;
};


#endif