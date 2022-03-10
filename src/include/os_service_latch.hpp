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

#ifndef _OS_SERVICE_LATCH_HPP__
#define _OS_SERVICE_LATCH_HPP__

#include "core.hpp"

#ifdef _WINDOWS
    #define os_service_mutex_t						                     CRITICAL_SECTION
    #define os_service_mutex_init(__lock, __attribute)	             	 InitializeCriticalSection( (__lock) )
    #define os_service_mutex_destroy				                     DeleteCriticalSection
    #define os_service_mutex_lock					                     EnterCriticalSection
    #define os_service_mutex_trylock(__lock)		                     (TRUE == TryEnterCriticalSection( (__lock) ) )
    #define os_service_mutex_unlock				                         LeaveCriticalSection

    #define os_service_rwlock_t					                         SRWLOCK
    #define os_service_rwlock_init(__lock, __attribute)		             InitializeSRWLock( (__lock) )
    #define os_service_rwlock_destroy(__lock)		                     (1)
    #define os_service_rwlock_rdlock				                     AcquireSRWLockShared
    #define os_service_rwlock_rdunlock				                     ReleaseSRWLockShared
    #define os_service_rwlock_wrlock				                     AcquireSRWLockExclusive
    #define os_service_rwlock_wrunlock				                     ReleaseSRWLockExclusive
    #define os_service_rwlock_rdtrylock(__lock)	                         (false)
    #define os_service_rwlock_wrtrylock(__lock)	                         (false)
#else
    #define os_service_mutex_t						                     pthread_mutex_t
    #define os_service_mutex_init					                     pthread_mutex_init
    #define os_service_mutex_destroy				                     pthread_mutex_destroy
    #define os_service_mutex_lock					                     pthread_mutex_lock
    #define os_service_mutex_trylock(__lock)	                         (pthread_mutex_trylock( (__lock) ) == 0 )
    #define os_service_mutex_unlock				                         pthread_mutex_unlock

    #define os_service_rwlock_t					                         pthread_rwlock_t
    #define os_service_rwlock_init					                     pthread_rwlock_init
    #define os_service_rwlock_destroy				                     pthread_rwlock_destroy
    #define os_service_rwlock_rdlock				                     pthread_rwlock_rdlock
    #define os_service_rwlock_rdunlock				                     pthread_rwlock_unlock
    #define os_service_rwlock_wrlock				                     pthread_rwlock_wrlock
    #define os_service_rwlock_wrunlock				                     pthread_rwlock_unlock
    #define os_service_rwlock_rdtrylock(__lock)	                         (pthread_rwlock_tryrdlock( (__lock) ) == 0 )
    #define os_service_rwlock_wrtrylock(__lock)	                         (pthread_rwlock_trywrlock ( ( __lock) ) == 0 )
#endif

enum OS_SERVICE_LATCH_MODE {
    SHARD,
    EXCLUSIVE
};

// 互斥锁
class OSServiceXLatch {
private:
    pthread_mutex_t _lock;
public:
    OSServiceXLatch();
    ~OSServiceXLatch();
    void get();
    void release();
    bool try_get();
};

// 共享锁
class OSServiceSLatch {
private:
    pthread_rwlock_t _lock;
public:
    OSServiceSLatch();
    ~OSServiceSLatch();
    void get();
    void release();
    bool try_get();
    bool try_get_shared();
    void get_shared();
    void release_shared();
};

#endif