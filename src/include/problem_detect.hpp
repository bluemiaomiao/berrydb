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

#ifndef _PROBLEM_DETECT_HPP__
#define _PROBLEM_DETECT_HPP__

#include "core.hpp"

#define PROBLEM_DETECT_LOG_STRING_MAX 4096

#define PROBLEM_DETECT_LOG(level, fmt, ...)                                                      \
   do {                                                                                          \
      if (curProblemDetectLevel >= level) {                                                      \
         problemDetectLog (level, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__);             \
      }                                                                                          \
   } while (0)                                                                                   \

#define PROBLEM_DETECT_CHECK(cond, retCode, gotoLabel, level, fmt, ...)                          \
   do {                                                                                          \
      if (!(cond)) {                                                                             \
         rc = (retCode);                                                                         \
         PROBLEM_DETECT_LOG ((level), fmt, ##__VA_ARGS__);                                       \
         goto gotoLabel ;                                                                        \
      }                                                                                          \
   } while (0)                                                                                   \

#define PROBLEM_DETECT_RC_CHECK(rc, level, fmt, ...)                                             \
   do {                                                                                          \
      PROBLEM_DETECT_CHECK ((DB_OK==(rc)), (rc), error, (level), fmt, ##__VA_ARGS__);            \
   } while (0)                                                                                   \

#define DB_VALIDATE_GOTO_ERROR(cond, ret, str) {                                                 \
   if(!(cond)) { problemDetectLog(PROBLEM_DETECT_ERROR, __func__, __FILE__, __LINE__, str);      \
    rc = ret; goto error; }}

#define DB_ASSERT(cond, str)  { if(cond){} }
#define DB_CHECK(cond, str)   { if(cond){} }

enum PROBLEM_DETECT_LEVEL {
    PROBLEM_DETECT_SEVERE = 0,
    PROBLEM_DETECT_ERROR,
    PROBLEM_DETECT_EVENT,
    PROBLEM_DETECT_WARNING,
    PROBLEM_DETECT_INFO,
    PROBLEM_DETECT_DEBUG
};

extern PROBLEM_DETECT_LEVEL curProblemDetectLevel;

const char *getProblemDetectLevelDescription(PROBLEM_DETECT_LEVEL level);

#define PROBLEM_DETECT_DFT_DIAG_LEVEL PROBLEM_DETECT_WARNING

void problemDetectLog(PROBLEM_DETECT_LEVEL level, const char *func, const char *file, unsigned int line,
                      const char *format, ...);
void problemDetectLog(PROBLEM_DETECT_LEVEL level, const char *func, const char *file, unsigned int line,
                      std::string message);

#endif