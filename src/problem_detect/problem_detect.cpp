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
#include "problem_detect.hpp"
#include "os_service_latch.hpp"
#include "os_service_primitive_file_op.hpp"

const static char *PROBLEM_DETECT_LEVEL_STRING[] =
{
    "SEVERE",
    "ERROR",
    "EVENT",
    "WARNING",
    "INFO",
    "DEBUG"
};

const char *getProblemDetectLevelDescription(PROBLEM_DETECT_LEVEL level) {
    if ((unsigned int) level > (unsigned int) PROBLEM_DETECT_DEBUG) {
        return "Unknown Level";
    }
    return PROBLEM_DETECT_LEVEL_STRING[(unsigned int) level];
}

#ifdef _WINDOWS

    const static char *PROBLEM_DETECT_LOG_HEADER_FORMAT="%04d-%02d-%02d-%02d.%02d.%02d.%06dLevel:%s" OS_SERVICE_NEWLINE "PID:%-37luTID:%lu" OS_SERVICE_NEWLINE "Function:%-32sLine:%u" OS_SERVICE_NEWLINE "File:%s" OS_SERVICE_NEWLINE "Message:" OS_SERVICE_NEWLINE "%s" OS_SERVICE_NEWLINE OS_SERVICE_NEWLINE ;

#else

    const static char *PROBLEM_DETECT_LOG_HEADER_FORMAT = "%04d-%02d-%02d-%02d.%02d.%02d.%06dLevel:%s" OS_SERVICE_NEWLINE "PID:%-37dTID:%d" OS_SERVICE_NEWLINE "Function:%-32sLine:%d" OS_SERVICE_NEWLINE "File:%s" OS_SERVICE_NEWLINE "Message:" OS_SERVICE_NEWLINE "%s" OS_SERVICE_NEWLINE OS_SERVICE_NEWLINE ;
#endif

PROBLEM_DETECT_LEVEL curProblemDetectLevel = PROBLEM_DETECT_DFT_DIAG_LEVEL;

char problemDetectDiagLogPath[OS_SERVICE_MAX_PATHSIZE + 1] = {0};

OSServiceXLatch problemDetectLogMutex;
OSServicePrimitiveFileOp problemDetectLogFile;

static int pdLogFileReopen() {
    int rc = DB_OK;
    problemDetectLogFile.Close();
    rc = problemDetectLogFile.Open(problemDetectDiagLogPath);
    if (rc) {
        printf("Failed to open log file, errno = %d" OS_SERVICE_NEWLINE, rc);
        goto error;
    }
    problemDetectLogFile.seekToEnd();
    done :
    return rc;
    error :
    goto done;
}

static int problemDetectLogFileWrite(const char *pData) {
    int rc = DB_OK;
    
    size_t dataSize = strlen(pData);
    
    problemDetectLogMutex.get();
    
    if (!problemDetectLogFile.isValid()) {
        rc = pdLogFileReopen();
        if (rc) {
            printf("Failed to open log file, errno = %d" OS_SERVICE_NEWLINE,
                   rc);
            goto error;
        }
    }
    
    rc = problemDetectLogFile.Write(pData, dataSize);
    if (rc) {
        printf("Failed to write into log file, errno = %d" OS_SERVICE_NEWLINE,
               rc);
        goto error;
    }
    
    done :
        problemDetectLogMutex.release();
        return rc;
    error :
        goto done;
}

void problemDetectLog(PROBLEM_DETECT_LEVEL level, const char *func, const char *file, unsigned int line,
                      const char *format, ...) {
    int rc = DB_OK;

    if (curProblemDetectLevel < level) {
        return;
    }

    va_list ap;
    char userInfo[PROBLEM_DETECT_LOG_STRING_MAX];
    char sysInfo[PROBLEM_DETECT_LOG_STRING_MAX];

    va_start (ap, format);
    vsnprintf(userInfo, PROBLEM_DETECT_LOG_STRING_MAX, format, ap);
    va_end (ap);

#ifdef _WINDOWS
    SYSTEMTIME systime;
    GetLocalTime(&systime);

    // %04d-%02d-%02d-%02d.%02d.%02d.%06d
    snprintf(sysInfo, PROBLEM_DETECT_LOG_STRING_MAX, PROBLEM_DETECT_LOG_HEADER_FORMAT, systime.wYear, systime.wMonth,
             systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds * 1000,
             PROBLEM_DETECT_LEVEL_STRING[level], getpid(), pthread_self(), func, line, file, userInfo);

#else
    struct tm otm;
    struct timeval tv;
    struct timezone tz;
    time_t tt;

    gettimeofday(&tv, &tz);
    tt = tv.tv_sec;
    localtime_r(&tt, &otm);

    snprintf(sysInfo, PROBLEM_DETECT_LOG_STRING_MAX, PROBLEM_DETECT_LOG_HEADER_FORMAT,
             otm.tm_year + 1900, otm.tm_mon + 1, otm.tm_mday, otm.tm_hour, otm.tm_min, otm.tm_sec, tv.tv_usec,
             PROBLEM_DETECT_LEVEL_STRING[level], getpid(), syscall(SYS_gettid), func, line, file, userInfo);
#endif
    printf("%s" OS_SERVICE_NEWLINE, sysInfo);

    if (problemDetectDiagLogPath[0] != '\0') {

        rc = problemDetectLogFileWrite(sysInfo);
        if (rc) {
            printf("Failed to write into log file, errno = %d" OS_SERVICE_NEWLINE, rc);
            printf("%s" OS_SERVICE_NEWLINE, sysInfo);
        }
    }
}