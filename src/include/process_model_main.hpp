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

#ifndef _PROCESS_MODEL_MAIN_HPP__
#define _PROCESS_MODEL_MAIN_HPP__

#include "core.hpp"
#include "process_model.hpp"
#include "process_model_options.hpp"
#include "problem_detect.hpp"
#include "process_model_engine_dispatch_unit_manager.hpp"

static int processModelResolveArguments ( int argc, char **argv );
int processModelMasterThreadMain (int argc, char **argv );
int main ( int argc, char **argv );

struct SignalInfo {
    const char *name ;
    int       handle ;
} ;

typedef struct SignalInfo signalInfo ;

static signalInfo signalHandleMap [] = {
        { "Unknow", 0 },
        { "SIGHUP", 1 },     //1
        { "SIGINT", 1 },     //2
        { "SIGQUIT", 1 },    //3
        { "SIGILL", 1 },     //4
        { "SIGTRAP", 1 },    //5
        { "SIGABRT", 1 },    //6
        { "SIGBUS", 1 },     //7
        { "SIGFPE", 1 },     //8
        { "SIGKILL", 1 },    //9
        { "SIGUSR1", 0 },    //10
        { "SIGSEGV", 1 },    //11
        { "SIGUSR2", 0 },    //12
        { "SIGPIPE", 1 },    //13
        { "SIGALRM", 0 },    //14
        { "SIGTERM", 1 },    //15
        { "SIGSTKFLT", 0 },  //16
        { "SIGCHLD", 0 },    //17
        { "SIGCONT", 0 },    //18
        { "SIGSTOP", 1 },    //19
        { "SIGTSTP", 0 },    //20
        { "SIGTTIN", 0 },    //21
        { "SIGTTOU", 0 },    //22
        { "SIGURG", 0 },     //23
        { "SIGXCPU", 0 },    //24
        { "SIGXFSZ", 0 },    //25
        { "SIGVTALRM", 0 },  //26
        { "SIGPROF", 0 },    //27
        { "SIGWINCH", 0 },   //28
        { "SIGIO", 0 },      //29
        { "SIGPWR", 1 },     //30
        { "SIGSYS", 1 },     //31
        { "UNKNOW", 0 },     //32
        { "UNKNOW", 0 },     //33
        { "SIGRTMIN", 0 },   //34
        { "SIGRTMIN+1", 0 }, //35
        { "SIGRTMIN+2", 0 }, //36
        { "SIGRTMIN+3", 0 }, //37
        { "SIGRTMIN+4", 0 }, //38
        { "SIGTTMIN+5", 0 }, //39
        { "SIGRTMIN+6", 0 }, //40
        { "SIGRTMIN+7", 0 }, //41
        { "SIGTTMIN+8", 0 }, //42
        { "SIGRTMIN+9", 0 }, //43
        { "SIGRTMIN+10", 0 },//44
        { "SIGRTMIN+11", 0 },//45
        { "SIGRTMIN+12", 0 },//46
        { "SIGRTMIN+13", 0 },//47
        { "SIGRTMIN+14", 0 },//48
        { "SIGRTMIN+15", 0 },//49
        { "SIGRTMAX-14", 0 },//50
        { "SIGRTMAX-13", 0 },//51
        { "SIGRTMAX-12", 0 },//52
        { "SIGRTMAX-11", 0 },//53
        { "SIGRTMAX-10", 0 },//54
        { "SIGRTMAX-9", 0 }, //55
        { "SIGRTMAX-8", 0 }, //56
        { "SIGRTMAX-7", 0 }, //57
        { "SIGRTMAX-6", 0 }, //58
        { "SIGRTMAX-5", 0 }, //59
        { "SIGRTMAX-4", 0 }, //60
        { "SIGRTMAX-3", 0 }, //61
        { "SIGRTMAX-2", 0 }, //62
        { "SIGRTMAX-1", 0 }, //63
        { "SIGRTMAX", 0 },   //64
};

#define PROCESS_MODEL_MAX_SIGNALS 64

#ifndef _WINDOWS
    static void processModelSignalHandler ( int sigNum );
#else
    static BOOL WINAPI processModelSignalHandler(DWORD dwCtrlType);
#endif

#ifndef _WINDOWS
    static int processModelSetupSignalHandler ();
#endif

#endif