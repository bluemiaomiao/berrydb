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

#include "core.hpp"
#include "os_service_primitive_file_op.hpp"

OSServicePrimitiveFileOp::OSServicePrimitiveFileOp() {
    this->_fileHandle = OS_SERVICE_INVALID_HANDLE_FD_VALUE;
    this->_bIsStdout = false;
}

bool OSServicePrimitiveFileOp::isValid() const {
    return (OS_SERVICE_INVALID_HANDLE_FD_VALUE != this->_fileHandle);
}

void OSServicePrimitiveFileOp::Close() {
    if (this->isValid() && (!this->_bIsStdout)) {
        os_service_close(this->_fileHandle);
        this->_fileHandle = OS_SERVICE_INVALID_HANDLE_FD_VALUE;
    }
}

int OSServicePrimitiveFileOp::Open(const char *pFilePath, unsigned int options) {
    int rc = 0;
    int mode = O_RDWR;

    if (options & OS_SERVICE_PRIMITIVE_FILE_OP_READ_ONLY) {
        mode = O_RDONLY;
    } else if (options & OS_SERVICE_PRIMITIVE_FILE_OP_WRITE_ONLY) {
        mode = O_WRONLY;
    }

    if (options & OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_EXISTING) {
    } else if (options & OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_ALWAYS) {
        mode |= O_CREAT;
    }
    if (options & OS_SERVICE_PRIMITIVE_FILE_OP_OPEN_TRUNC) {
        mode |= O_TRUNC;
    }

    do {
        this->_fileHandle = os_service_open(pFilePath, mode, 0644);
    } while ((this->_fileHandle == -1) && (errno == EINTR));

    if (this->_fileHandle <= OS_SERVICE_INVALID_HANDLE_FD_VALUE) {
        rc = errno;
        goto exit;
    }

    exit:
        return rc;
}

void OSServicePrimitiveFileOp::openStdout() {
    this->setFileHandle(STDOUT_FILENO);
    this->_bIsStdout = true;
}

offsetType OSServicePrimitiveFileOp::getCurrentOffset() const {
    return os_service_lseek(this->_fileHandle, 0, SEEK_CUR);
}

void OSServicePrimitiveFileOp::seekToEnd(void) const {
    os_service_lseek(this->_fileHandle, 0, SEEK_END);
}

void OSServicePrimitiveFileOp::seekToOffset(offsetType offset) {
    if ((os_service_off_t) -1 != offset) {
        os_service_lseek(this->_fileHandle, offset, SEEK_SET);
    }
}

int OSServicePrimitiveFileOp::Read(const size_t size, void *const pBuffer, int *const pBytesRead) {
    int retVal = 0;
    ssize_t bytesRead = 0;

    if (this->isValid()) {
        do {
            bytesRead = os_service_read(this->_fileHandle, pBuffer, size);
        } while ((bytesRead == -1) && (errno == EINTR));

        if (-1 == bytesRead) {
            goto err_read;
        }
    } else {
        goto err_read;
    }

    if (pBytesRead) {
        *pBytesRead = bytesRead;
    }

    exit:
        return retVal;
    err_read:
        *pBytesRead = 0;
        retVal = errno;

    goto exit;
}

int OSServicePrimitiveFileOp::Write(const void *pBuffer, size_t size) const {
    int rc = 0;
    size_t currentSize = 0;

    if (size == 0) {
        size = strlen((char *) pBuffer);
    }

    if (this->isValid()) {
        do {
            rc = os_service_write(this->_fileHandle, &((char *) pBuffer)[currentSize],
                                  size - currentSize);
            if (rc >= 0)
                currentSize += rc;
        } while (((rc == -1) && (errno == EINTR)) ||
                 ((rc != -1) && (currentSize != size)));
        if (rc == -1) {
            rc = errno;
            goto exit;
        }
        rc = 0;
    }

    exit:
        return rc;
}

int OSServicePrimitiveFileOp::fWrite(const char *format, ...) {
    int rc = 0;
    va_list ap;

    char buf[OS_SERVICE_PRIMITIVE_FILE_OP_FWRITE_BUF_SIZE] = {0};

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    rc = this->Write(buf);

    return rc;
}

void OSServicePrimitiveFileOp::setFileHandle(handleType handle) {
    this->_fileHandle = handle;
}

int OSServicePrimitiveFileOp::getSize(offsetType *const pFileSize) {
    int rc = 0;
    os_service_struct_stat buf = {0};

    if (os_service_fstat(this->_fileHandle, &buf) == -1) {
        rc = errno;
        goto err_exit;
    }

    *pFileSize = buf.st_size;

    exit:
        return rc;
    err_exit:
        *pFileSize = 0;

    goto exit;
}

OSServicePrimitiveFileOp::handleType OSServicePrimitiveFileOp::getHandle() const {
    return this->_fileHandle;
}
