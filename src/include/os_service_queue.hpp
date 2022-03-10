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

#ifndef _OS_SERVICE_QUEUE_HPP__
#define _OS_SERVICE_QUEUE_HPP__


#include <queue>
#include <boost/thread.hpp>
#include <boost/thread/thread_time.hpp>

template<typename Data>
class OSServiceQueue {
private:
    std::queue<Data> _q;
    boost::mutex _m;
    boost::condition_variable _cond;
public:
    unsigned int size();
    void push(Data data);
    bool empty() const;
    bool try_pop(Data &value);
    void wait_and_pop(Data &value);
    bool time_wait_and_pop(Data &value, long long millsec);
};

#endif