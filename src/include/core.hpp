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

#ifndef _CORE_HPP__
#define _CORE_HPP__

#ifdef _WINDOWS
    #include "database_windows.h"
#else
    #include <unistd.h>
    #include <linux/limits.h>
    #include <sys/time.h>
    #include <syscall.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <signal.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <netinet/tcp.h>
    #include <sys/mman.h>
    #include <pthread.h>
#endif

#include <errno.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cstdarg>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <iostream>

#define OS_SERVICE_MAX_PATHSIZE                            PATH_MAX    // 路径最大长度
#if defined _WINDOWS
    #define OS_SERVICE_FILE_SEP_STR                            "\\"
#else
    #define OS_SERVICE_FILE_SEP_STR                            "/"     // 路径分隔符
#endif

#define OS_SERVICE_FILE_SEP_CHR                  *((const char*)OS_SERVICE_FILE_SEP_STR)[0]   // 路径分隔符
#define OS_SERVICE_NEWLINE                       "\n"                                         // 仅支持Unix/Linux

#define DB_OK                                               0        // 成功
#define DB_IO                                              -1        // I/O错误
#define DB_INVALID_ARG                                     -2        // 非法参数
#define DB_PERMIT                                          -3        // 权限错误
#define DB_OOM                                             -4        // 内存不足
#define DB_SYS                                             -5        // 系统错误
#define DB_PROCESS_MODEL_HELP_ONLY                         -6        // 
#define DB_PROCESS_MODEL_FORCE_SYS_ENGINE_DISPATCH_UNIT    -7        //
#define DB_TIMEOUT                                         -8        // 超时
#define DB_QUIESCED                                        -9        // 
#define DB_ENGINE_DISPATCH_UNIT_INVALID_STATUS             -10       // ENGINE_DISPATCH_UNIT进入非法状态
#define DB_NETWORK                                         -11       // 网络错误
#define DB_NETWORK_CLOSE                                   -12       // 网络关闭
#define DB_APP_FORCED                                      -13       //
#define DB_INDEX_MANAGEMENT_ID_EXIST                       -14       // 索引管理器ID已经存在
#define DB_HEADER_INVALID                                  -15       // 非法元数据
#define DB_INDEX_MANAGEMENT_ID_NOT_EXIST                   -16       // 索引管理器ID不存在
#define DB_NO_ID                                           -17       // 找不到ID

#endif
