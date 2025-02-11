//
// Created by lqf on 23-3-31.
//

#ifndef DEFTRPC_LOG_H
#define DEFTRPC_LOG_H

#include "Appender/LogAppender.h"
#include "LogCommon.h"
#include "LogRecord.h"
#include "LoggerBase.h"
#include "common/thread.h"

#define CLSN_LOGGER_DEFAULT_ID 0

#define CLSN_LEVEL_CHECK(Id, Level)                                 \
  if (static_cast<std::uint16_t>(clsn::LogLevel::Level) <=          \
      static_cast<std::uint16_t>(clsn::GetLogger<Id>().GetLevel())) \
    ;                                                               \
  else                                                              \
    clsn::GetLogger<Id>() +=

#define CLSN_DEFAULT_LEVEL_CHECK(Level) CLSN_LEVEL_CHECK(CLSN_LOGGER_DEFAULT_ID, Level)

#define CLSN_DO_LOG(Id, Level)                                                                     \
  CLSN_LEVEL_CHECK(Id, Level)                                                                      \
  clsn::LogRecord(&clsn::GetLogger<Id>(), ::time(NULL), __FILE__, __LINE__, clsn::LogLevel::Level, \
                  clsn::thread::ThisThreadId())

#define CLSN_DEFAULT_DO_LOG(Level) CLSN_DO_LOG(CLSN_LOGGER_DEFAULT_ID, Level)

#define CLSN_LOG_NONE CLSN_DEFAULT_DO_LOG(None)
// #ifdef Debug
#define CLSN_LOG_DEBUG CLSN_DEFAULT_DO_LOG(Debug)
// #else
// #define CLSN_LOG_DEBUG  //
// #endif
#define CLSN_LOG_ERROR CLSN_DEFAULT_DO_LOG(Error)
#define CLSN_LOG_FATAL CLSN_DEFAULT_DO_LOG(Fatal)
#define CLSN_LOG_WARNING CLSN_DEFAULT_DO_LOG(Warning)
#define CLSN_LOG_INFO CLSN_DEFAULT_DO_LOG(INFO)

#define CLSN_LOG_NONE_(Id) CLSN_DO_LOG(Id, None)
#define CLSN_LOG_DEBUG_(Id) CLSN_DO_LOG(Id, Debug)
#define CLSN_LOG_ERROR_(Id) CLSN_DO_LOG(Id, Warning)
#define CLSN_LOG_FATAL_(Id) CLSN_DO_LOG(Id, Error)
#define CLSN_LOG_WARNING_(Id) CLSN_DO_LOG(Id, Fatal)

#endif  // DEFTRPC_LOG_H
