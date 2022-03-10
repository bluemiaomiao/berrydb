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

#ifndef _OS_SERVICE_MMAP_FILE_HPP__
#define _OS_SERVICE_MMAP_FILE_HPP__

#include "core.hpp"
#include "os_service_latch.hpp"
#include "os_service_primitive_file_op.hpp"

class _OSServiceMmapFile {
protected:
    class _osServiceMmapSegment {
    public:
        void* _ptr;
        unsigned int _length;
        unsigned long long _offset;
        _osServiceMmapSegment(void* ptr, unsigned int length, unsigned long long offset) {
            this->_ptr = ptr;
            this->_length = length;
            this->_offset = offset;
        }
    };
    typedef _osServiceMmapSegment OSServiceMmapSegment;
    OSServicePrimitiveFileOp _fileOp;
    OSServiceXLatch _mutex;
    bool _opened;
    std::vector<OSServiceMmapSegment> _segment;
    char _fileName[OS_SERVICE_MAX_PATHSIZE];
public:
    typedef std::vector<OSServiceMmapSegment>::const_iterator CONST_ITR;
    inline CONST_ITR begin() { return this->_segment.begin(); }
    inline CONST_ITR end() { return this->_segment.end(); }
    inline unsigned int segmentSize() { return this->_segment.size(); }
    _OSServiceMmapFile();
    ~_OSServiceMmapFile();
    int open(const char* pFilename, unsigned int options);
    int map(unsigned long long offset, unsigned int length, void** pAddress);
    void close();
};

typedef class _OSServiceMmapFile OSServiceMmapFile;

#endif