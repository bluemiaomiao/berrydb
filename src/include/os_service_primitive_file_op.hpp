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

#ifndef _OS_SERVICE_PRIMITIVE_FILE_OP__
#define _OS_SERVICE_PRIMITIVE_FILE_OP__

#include "core.hpp"

#ifdef _WINDOWS
    #include <io.h>
    #include <fcntl.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    
    #define OS_SERVICE_F_GETLK                        F_GETLK64
    #define OS_SERVICE_F_SETLK                        F_SETLK64
    #define OS_SERVICE_F_SETLKW                       F_SETLKW64
    
    #define os_service_struct_statfs                  struct statfs64
    #define os_service_statfs                         statfs64
    #define os_service_fstatfs                        fstatfs64
    #define os_service_struct_statvfs                 struct statvfs64
    #define os_service_statvfs                        statvfs64
    #define os_service_fstatvfs                       fstatvfs64
    #define os_service_struct_stat                    struct _stati64
    #define os_service_struct_flock                   struct flock64
    #define os_service_stat                           stat64
    #define os_service_lstat                          lstat64
    #define os_service_fstat                          _fstati64
    #define os_service_open                           _open
    #define os_service_lseek                          _lseeki64
    #define os_service_ftruncate                      ftruncate64
    #define os_service_off_t                          __int64
    #define os_service_close                          _close
    #define os_service_access                         access
    #define os_service_chmod
    #define os_service_read                           read
    #define os_service_write                          write
    
    #define O_RDWR				                      _O_RDWR
    #define O_RDONLY			                      _O_RDONLY
    #define O_WRONLY			                      _O_WRONLY
    #define O_CREAT			  	                      _O_CREAT
    #define O_TRUNC				                      _O_TRUNC
    
    #define OS_SERVICE_HANDLE			              int
    #define OS_SERVICE_INVALID_HANDLE_FD_VALUE        (OS_SERVICE_HANDLE(-1))
#else
    #define OS_SERVICE_HANDLE		                  int
    #define OS_SERVICE_F_GETLK                        F_GETLK64
    #define OS_SERVICE_F_SETLK                        F_SETLK64
    #define OS_SERVICE_F_SETLKW                       F_SETLKW64
    
    #define os_service_struct_statfs                  struct statfs64
    #define os_service_statfs                         statfs64
    #define os_service_fstatfs                        fstatfs64
    #define os_service_struct_statvfs                 struct statvfs64
    #define os_service_statvfs                        statvfs64
    #define os_service_fstatvfs                       fstatvfs64
    #define os_service_struct_stat                    struct stat64
    #define os_service_struct_flock                   struct flock64
    #define os_service_stat                           stat64
    #define os_service_lstat                          lstat64
    #define os_service_fstat                          fstat64
    #define os_service_open                           open64
    #define os_service_lseek                          lseek64
    #define os_service_ftruncate                      ftruncate64
    #define os_service_off_t                          off64_t
    #define os_service_close                          close
    #define os_service_access                         access
    #define os_service_chmod                          chmod
    #define os_service_read                           read
    #define os_service_write                          write
    
    #define OS_SERVICE_INVALID_HANDLE_FD_VALUE        (-1)
#endif

#define OS_SERVICE_PRIMITIVE_FILE_OP_FWRITE_BUF_SIZE  2048
#define OS_SERVICE_PRIMITIVE_FILE_OP_READ_ONLY        (((unsigned int)1) << 1)
#define OS_SERVICE_PRIMITIVE_FILE_OP_WRITE_ONLY       (((unsigned int)1) << 2)
#define OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_EXISTING    (((unsigned int)1) << 3)
#define OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_ALWAYS      (((unsigned int)1) << 4)
#define OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_TRUNC       (((unsigned int)1) << 5)

typedef os_service_off_t offsetType;

class OSServicePrimitiveFileOp {
public:
    typedef  OS_SERVICE_HANDLE handleType;
    OSServicePrimitiveFileOp();
    int Open(const char* pFilePath, unsigned int options = OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_ALWAYS);
    void openStdout();
    void Close();
    bool isValid() const;
    int Read(const size_t, void* const pBuf, int* const pBytesRead);
    int Write(const void* pBuf, size_t len = 0) const;
    int fWrite(const char* fmt, ...);
    offsetType getCurrentOffset() const;
    void seekToOffset(offsetType offset);
    void seekToEnd() const;
    int getSize(offsetType* const pFileSize);
    handleType getHandle() const;
private:
    handleType _fileHandle;
    OSServicePrimitiveFileOp(const OSServicePrimitiveFileOp &op);
    bool _bIsStdout;
protected:
    void setFileHandle(handleType handle);
};

#endif