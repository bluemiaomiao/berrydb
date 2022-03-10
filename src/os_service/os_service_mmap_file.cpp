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

#include "os_service_mmap_file.hpp"
#include "problem_detect.hpp"

using namespace std ;

int OSServiceMmapFile::open ( const char * pFilename, unsigned int options ) {
    int rc = DB_OK ;
    this->_mutex.get() ;

    rc = this->_fileOp.Open(pFilename, options);
    if (DB_OK == rc) {
        this->_opened = true ;
    } else {
        PROBLEM_DETECT_LOG (PROBLEM_DETECT_ERROR, "Failed to open file, rc = %d", rc);
        goto error ;
    }

    strncpy(this->_fileName, pFilename, sizeof(this->_fileName));

    done :
        this->_mutex.release();
        return rc ;
    error :
        goto done ;
}

void OSServiceMmapFile::close () {
    this->_mutex.get() ;

    for (vector<OSServiceMmapSegment>::iterator i = this->_segment.begin(); i != this->_segment.end(); i++ ) {
        munmap((void*)(*i)._ptr, (*i)._length);
    }

    this->_segment.clear();

    if (this->_opened) {
        this->_fileOp.Close () ;
        this->_opened = false ;
    }

    this->_mutex.release () ;
}

int OSServiceMmapFile::map (unsigned long long offset, unsigned int length, void **pAddress) {
    this->_mutex.get() ;
    int rc = DB_OK ;

    OSServiceMmapSegment seg (0, 0, 0) ;

    unsigned long long fileSize = 0 ;

    void *segment = NULL ;

    if (length == 0) {
        goto done ;
    }

    rc = this->_fileOp.getSize((off_t*)&fileSize);
    if (rc) {
        PROBLEM_DETECT_LOG(PROBLEM_DETECT_ERROR, "Failed to get file size, rc = %d", rc ) ;
        goto error ;
    }

    if (offset + length > fileSize) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Offset is greater than file size" ) ;
        rc = DB_INVALID_ARG ;
        goto error ;
    }

    segment = mmap(NULL, length, PROT_READ | PROT_WRITE,MAP_SHARED, this->_fileOp.getHandle(),
                   offset);

    if (segment == MAP_FAILED) {
        PROBLEM_DETECT_LOG ( PROBLEM_DETECT_ERROR, "Failed to map offset %ld length %d, errno = %d", offset, length,
                             errno);

        if (errno == ENOMEM ) {
            rc = DB_OOM;
        } else if (errno == EACCES) {
            rc = DB_PERMIT;
        } else {
            rc = DB_SYS;
        }

        goto error ;
    }

    seg._ptr = segment;
    seg._length = length;
    seg._offset = offset;

    this->_segment.push_back(seg);

    if (pAddress) {
        *pAddress = segment ;
    }

    done :
        this->_mutex.release () ;
        return rc ;
    error :
        goto done ;
}

//_OSServiceMmapFile::CONST_ITR _OSServiceMmapFile::begin() {
//    return this->_segment.begin();
//}

//_OSServiceMmapFile::CONST_ITR _OSServiceMmapFile::end() {
//    return this->_segment.end();
//}

//unsigned int _OSServiceMmapFile::segmentSize() {
//    return this->_segment.size();
//}

_OSServiceMmapFile::_OSServiceMmapFile() {
    this->_opened = false;
    memset(this->_fileName, 0, sizeof(this->_fileName));
}

_OSServiceMmapFile::~_OSServiceMmapFile() {
    this->close();
}
