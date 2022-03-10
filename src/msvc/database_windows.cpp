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

#ifdef _WINDOWS

#include "core.hpp"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef WIN32
#pragma comment(lib, "ws2_32.lib")
#else
#error("do not the appropriate library");
#endif

const int SEC_PER_MSEC = 1000;

unsigned sleep(unsigned seconds)
{
    Sleep(seconds * SEC_PER_MSEC);
    return 0;
}

unsigned getpid(void)
{
    return (unsigned)GetCurrentProcessId();
}

unsigned pthread_self()
{
    return (unsigned)GetCurrentThreadId();
}

struct tm *localtime_r(const time_t *timer, struct tm *result)
{
    if (0 == localtime_s(result, timer) )
    {
        return result;
    }
    return NULL;
}

class __CSocketEnvironment
{
public:
    __CSocketEnvironment()
    {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }

    ~__CSocketEnvironment()
    {
        WSACleanup();
    }
};

static __CSocketEnvironment		sgl_SocketEnvironment;

#ifdef __cplusplus
};
#endif

#endif