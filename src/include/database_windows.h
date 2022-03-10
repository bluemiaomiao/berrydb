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

#ifndef _DATABASE_WINDOWS_H__
#define _DATABASE_WINDOWS_H__

#ifdef _WINDOWS

#include <ws2tcpip.h>
#include <windows.h>
#include <WinBase.h>
#include <limits.h>

#define PATH_MAX	512
#define __func__	__FUNCTION__
#define STDOUT_FILENO	0
#define MSG_NOSIGNAL	0
#define EWOULDBLOCK		WSAEWOULDBLOCK

typedef __int64		ssize_t;
typedef __int32		socklen_t;

#ifdef __cplusplus
extern "C" {
#endif

unsigned sleep(unsigned seconds);
unsigned getpid(void);
unsigned pthread_self();
struct tm *localtime_r(const time_t *timer, struct tm *result);

#define snprintf	sprintf_s

#endif

#ifdef __cplusplus
};
#endif

#endif