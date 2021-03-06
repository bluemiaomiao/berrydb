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

#ifndef _OS_SERVICE_UTIL_HPP__
#define _OS_SERVICE_UTIL_HPP__

#include "core.hpp"
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/xtime.hpp>

#if defined _WINDOWS
    #include <TlHelp32.h>
#endif

#if defined _WINDOWS
    inline void OSServiceSleepMicros(unsigned int s) {
       boost::xtime xt ;
       boost::xtime_get(&xt, boost::TIME_UTC_) ;

       xt.sec += (int)(s / 1000000) ;
       xt.nsec += (int)((s % 1000000) * 1000) ;

       if(xt.nsec >= 1000000000) {
          xt.nsec -= 1000000000;
          xt.sec++;
       }

       boost::thread::sleep(xt);
    }
#else
    inline void OSServiceSleepMicros(unsigned int s) {
        struct timespec t;
        t.tv_sec = (time_t) (s / 1000000);
        t.tv_nsec = 1000 * (s % 1000000);
        while(nanosleep(&t, &t) == -1 && errno == EINTR);
    }
#endif

inline void OSServiceSleepMillis(unsigned int s) {
    struct timespec t ;
    t.tv_sec = (time_t) (s / 1000000) ;
    t.tv_nsec = 1000 * (s % 1000000 ) ;
    while ( nanosleep ( &t, &t ) == -1 && errno == EINTR ) ;
}

#if defined _WINDOWS
    typedef DWORD OSSPID ;
    typedef DWORD OSSTID ;
#else
    typedef pid_t OSSPID ;
    typedef pthread_t OSSTID ;
#endif

inline OSSPID OSServiceParentProcessID() {
#if defined _WINDOWS
    OSSPID pid = -1 ;
    OSSPID ppid = -1 ;
    HANDLE hSnapshot = 0 ;
    PROCESSENTRY32 pe = {0} ;
    pe.dwSize = sizeof(PROCESSENTRY32) ;
    hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 ) ;
    if ( hSnapshot == 0 ) {
        goto error ;
    }
    pid = GetCurrentProcessId () ;
    if ( Process32First ( hSnapshot, &pe ) ) {
       do {
           if ( pe.th32ProcessID == pid ) {
               ppid = pe.th32ParentProcessID ;
               goto done ;
           }
       } while ( Process32Next ( hSnapshot, &pe ) ) ;
    }
    done:
       return ppid ;
    error :
       goto done ;
#else
    return getppid () ;
#endif
}
inline OSSPID OSServiceGetCurrentProcessID() {
#if defined _WINDOWS
    return GetCurrentProcessId () ;
#else
    return getpid () ;
#endif
}
inline OSSPID OSServiceGetCurrentThreadID() {
#if defined _WINDOWS
    return GetCurrentThreadId () ;
#else
    return syscall(SYS_gettid) ;
#endif
}


#endif
