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

#include "process_model_options.hpp"
#include "problem_detect.hpp"

ProcessModelOptions::ProcessModelOptions () {
    memset ( _dbPath, 0, sizeof(_dbPath) ) ;
    memset ( _logPath, 0, sizeof(_logPath) ) ;
    memset ( _confPath, 0, sizeof(_confPath) ) ;
    memset ( _svcName, 0, sizeof(_svcName) ) ;
    _maxPool = NUM_POOL ;
}

//ProcessModelOptions::~ProcessModelOptions () = default;

int ProcessModelOptions::readCmd ( int argc, char **argv, po::options_description &desc, po::variables_map &vm ) {
    int rc = DB_OK ;
    try {
        po::store ( po::command_line_parser(argc, argv).options (desc).allow_unregistered().run(), vm );
        po::notify ( vm ) ;
    } catch (po::unknown_option &e) {
        std::cerr << "Unknown arguments: " << e.get_option_name() << std::endl ;
        rc = DB_INVALID_ARG ;
        goto error ;
    } catch ( po::invalid_option_value &e ) {
        std::cerr << "Invalid arguments: " << e.get_option_name() << std::endl ;
        rc = DB_INVALID_ARG ;
        goto error ;
    } catch ( po::error &e ) {
        std::cerr << "Error: " << e.what() << std::endl ;
        rc = DB_INVALID_ARG ;
        goto error ;
    }

    done :
        return rc ;
    error :
        goto done ;
}

int ProcessModelOptions::importVM ( const po::variables_map &vm, bool isDefault ) {
    int rc = DB_OK ;
    const char *p = NULL ;

    if ( vm.count ( PROCESS_MODEL_OPTION_CONFPATH ) ){
        p = vm[PROCESS_MODEL_OPTION_CONFPATH].as<string>().c_str() ;
        strncpy ( _confPath, p, OS_SERVICE_MAX_PATHSIZE ) ;
    } else if ( isDefault ) {
        strcpy ( _confPath, "./" CONFIG_FILENAME ) ;
    }

    if ( vm.count ( PROCESS_MODEL_OPTION_LOGPATH ) ) {
        p = vm[PROCESS_MODEL_OPTION_LOGPATH].as<string>().c_str() ;
        strncpy ( _logPath, p, OS_SERVICE_MAX_PATHSIZE ) ;
    } else if ( isDefault ) {
        strcpy ( _logPath, "./" LOG_FILENAME ) ;
    }

    if ( vm.count ( PROCESS_MODEL_OPTION_DBPATH ) ) {
        p = vm[PROCESS_MODEL_OPTION_DBPATH].as<string>().c_str() ;
        strncpy ( _dbPath, p, OS_SERVICE_MAX_PATHSIZE ) ; 
    } else if ( isDefault ) {
        strcpy ( _dbPath, "./" DB_FILENAME ) ;
    }

    if ( vm.count ( PROCESS_MODEL_OPTION_SVCNAME ) ) {
        p = vm[PROCESS_MODEL_OPTION_SVCNAME].as<string>().c_str() ;
        strncpy ( _svcName, p, NI_MAXSERV ) ;
    } else if ( isDefault ) {
        strcpy ( _svcName, SVC_NAME ) ;
    }

    if ( vm.count ( PROCESS_MODEL_OPTION_MAXPOOL ) ) {
        _maxPool = vm [ PROCESS_MODEL_OPTION_MAXPOOL ].as<unsigned int> () ;
    } else if ( isDefault ) {
        _maxPool = NUM_POOL ;
    }

    return rc ;
}

int ProcessModelOptions::readConfigureFile ( const char *path, po::options_description &desc, po::variables_map &vm ) {
    int rc = DB_OK ;
    char conf[OS_SERVICE_MAX_PATHSIZE+1] = {0} ;
    strncpy ( conf, path, OS_SERVICE_MAX_PATHSIZE ) ;

    try {
        po::store ( po::parse_config_file<char> ( conf, desc, true ), vm ) ;
        po::notify ( vm ) ;
    } catch( po::reading_file ) {
        std::cerr << "Failed to open config file: " <<( std::string ) conf << std::endl
                  << "Using default settings" << std::endl ;
        rc = DB_IO ;
        goto error ;
    } catch ( po::unknown_option &e ) {
        std::cerr << "Unkown config element: " << e.get_option_name () << std::endl ;
        rc = DB_INVALID_ARG ;
        goto error ;
    } catch ( po::invalid_option_value &e ) {
        std::cerr << ( std::string ) "Invalid config element: " << e.get_option_name () << std::endl ;
        rc = DB_INVALID_ARG ;
        goto error ;
    } catch( po::error &e ) {
        std::cerr << e.what () << std::endl ;
        rc = DB_INVALID_ARG ;
        goto error;
    }

    done :
        return rc ;
    error :
        goto done ;
}

int ProcessModelOptions::init ( int argc, char **argv ) {
    int rc = DB_OK ;
    po::options_description all ( "Command options" ) ;
    po::variables_map vm ;
    po::variables_map vm2 ;

    PROCESS_MODEL_ADD_PARAM_OPTIONS_BEGIN( all )
    PROCESS_MODEL_COMMANDS_OPTIONS;
    PROCESS_MODEL_ADD_PARAM_OPTIONS_END

    rc = readCmd ( argc, argv, all, vm ) ;
    if ( rc ) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to read cmd, rc = %d", rc ) ;
        goto error ;
    }

    if ( vm.count ( PROCESS_MODEL_OPTION_HELP ) ) {
        std::cout << all << std::endl ;
        rc = DB_PROCESS_MODEL_HELP_ONLY ;
        goto done ;
    }

    if ( vm.count ( PROCESS_MODEL_OPTION_CONFPATH ) ) {
        rc = readConfigureFile ( vm[PROCESS_MODEL_OPTION_CONFPATH].as<string>().c_str(), all, vm2 ) ;
    }

    if ( rc ) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Unexpected error when reading conf file, rc = %d", rc ) ;
        goto error ;
    }

    rc = importVM ( vm2 ) ;
    if ( rc ) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to import from vm2, rc = %d", rc ) ;
        goto error ;
    }

    rc = importVM ( vm ) ;
    if ( rc ) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to import from vm, rc = %d", rc ) ;
        goto error ;
    }

    done :
        return rc ;
    error :
        goto done ;
}

//char *ProcessModelOptions::getDBPath() {
//    return this->_dbPath;
//}

//char *ProcessModelOptions::getLogPath() {
//    return this->_logPath;
//}

//char *ProcessModelOptions::getConfPath() {
//    return this->_confPath;
//}

//char *ProcessModelOptions::getServiceName() {
//    return this->_svcName;
//}

//int ProcessModelOptions::getMaxPool() {
//    return this->_maxPool;
//}

