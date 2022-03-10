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

#include "os_service_latch.hpp"

OSServiceXLatch::OSServiceXLatch() {
    pthread_mutex_init(&this->_lock, 0);
}

OSServiceXLatch::~OSServiceXLatch() {
    pthread_mutex_destroy(&this->_lock);
}

bool OSServiceXLatch::try_get() {
    return (pthread_mutex_trylock(&this->_lock) == 0);
}

void OSServiceXLatch::get() {
    pthread_mutex_lock(&this->_lock);
}

void OSServiceXLatch::release() {
    pthread_mutex_unlock(&this->_lock);
}

OSServiceSLatch::OSServiceSLatch() {
    pthread_rwlock_init(&this->_lock, 0);
}

OSServiceSLatch::~OSServiceSLatch() {
    pthread_rwlock_destroy(&this->_lock);
}

void OSServiceSLatch::get() {
    pthread_rwlock_wrlock(&this->_lock);
}

void OSServiceSLatch::get_shared() {
    pthread_rwlock_rdlock(&this->_lock);
}

bool OSServiceSLatch::try_get() {
    return (pthread_rwlock_trywrlock(&this->_lock) == 0);
}

bool OSServiceSLatch::try_get_shared() {
    return (pthread_rwlock_tryrdlock(&this->_lock) == 0);
}

void OSServiceSLatch::release(){
    pthread_rwlock_unlock(&this->_lock);
}

void OSServiceSLatch::release_shared() {
    pthread_rwlock_unlock(&this->_lock);
}