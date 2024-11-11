//
// Created by lqf on 23-3-30.
//

#ifndef DEFTRPC_LOGRECORD_H
#define DEFTRPC_LOGRECORD_H

#include <unistd.h>
#include <ctime>
#include <sstream>
#include "Convert/LogConvert.h"
#include "LogCommon.h"

namespace CLSN {

class LoggerBase;

class LogRecord {
 public:
  explicit LogRecord(const LoggerBase *logger, time_t time, std::string &&file, unsigned short line, LogLevel level,
                     pthread_t pid) noexcept;

  const LoggerBase *getLogger() const noexcept { return logger; }

  inline time_t getTime() const noexcept { return time; }

  inline const std::string &getFile() const noexcept { return file; }

  inline unsigned short getLine() const noexcept { return line; }

  inline LogLevel getLevel() const noexcept { return level; }

  inline pthread_t getPid() const noexcept { return pid; }

  inline std::stringstream &getMessage() noexcept { return message; }

  inline const std::stringstream &getMessage() const noexcept { return message; }

  //        template<typename T>
  //        inline std::stringstream &operator<<(const T &t) noexcept {
  //            LogConvert<T>::convert(message, t);
  //            return message;
  //        }

  template <typename T>
  LogRecord &&operator<<(const T &t) && noexcept {
    LogConvert<T>::convert(message, t);
    return std::move(*this);
  }

 private:
  const LoggerBase *logger;
  time_t time;
  std::string file;
  unsigned short line;
  LogLevel level;
  pthread_t pid;
  std::stringstream message;
};

}  // namespace CLSN

#endif  // DEFTRPC_LOGRECORD_H
