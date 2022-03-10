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

#include "os_service_queue.hpp"

template<typename Data>
unsigned int OSServiceQueue<Data>::size() {
    boost::mutex::scoped_lock lock(this->_m);
    return (unsigned int) this->_q.size();
}

template<typename Data>
void OSServiceQueue<Data>::push(Data data) {
    boost::mutex::scoped_lock lock(this->_m);
    this->_q.push(data);
    lock.unlock();
    this->_cond.notify_one();
}

template<typename Data>
bool OSServiceQueue<Data>::empty() const {
    boost::mutex::scoped_lock lock(this->_m);
    return this->_q.empty();
}

template<typename Data>
bool OSServiceQueue<Data>::try_pop(Data &value) {
    boost::mutex::scoped_lock lock(this->_m);
    if(this->_q.empty()) {
        return false;
    } else {
        value = this->_q.front();
        this->_q.pop();
        return true;
    }
}

template<typename Data>
void OSServiceQueue<Data>::wait_and_pop(Data &value) {
    boost::mutex::scoped_lock lock(this->_m);

    while (this->_q.empty()) {
        this->_cond.wait(lock);
    }

    value = this->_q.front();
    this->_q.pop ();
}

template<typename Data>
bool OSServiceQueue<Data>::time_wait_and_pop(Data &value, long long int millsec) {
    boost::system_time const timeout = boost::get_system_time() + boost::posix_time::milliseconds(millsec);
    boost::mutex::scoped_lock lock(this->_m);

    while (this->_q.empty()) {
        if (!this->_cond.timed_wait(lock, timeout)) {
            return false;
        }
    }

    value = this->_q.front();
    this->_q.pop();

    return true;
}
