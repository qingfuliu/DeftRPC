//
// Created by lqf on 23-3-30.
//

#ifndef DEFTRPC_LOGRECORD_H
#define DEFTRPC_LOGRECORD_H

#include <unistd.h>
#include <ctime>
#include <sstream>
#include <string>
#include <utility>
#include "Convert/LogConvert.h"
#include "LogCommon.h"

namespace clsn {

class LoggerBase;

class LogRecord {
 public:
  explicit LogRecord(const LoggerBase *logger, time_t time, std::string &&file, std::uint16_t line, LogLevel level,
                     pthread_t pid) noexcept;

  const LoggerBase *GetLogger() const noexcept { return m_logger_; }

  inline time_t GetTime() const noexcept { return m_time_; }

  inline const std::string &GetFile() const noexcept { return m_file_; }

  inline std::uint16_t GetLine() const noexcept { return m_line_; }

  inline LogLevel GetLevel() const noexcept { return m_level_; }

  inline pthread_t GetPid() const noexcept { return m_pid_; }

  inline std::stringstream &GetMessage() noexcept { return m_message_; }

  inline const std::stringstream &GetMessage() const noexcept { return m_message_; }

  //        template<typename T>
  //        inline std::stringstream &operator<<(const T &t) noexcept {
  //            LogConvert<T>::Convert(message, t);
  //            return message;
  //        }

  template <typename T>
  LogRecord &&operator<<(const T &t) && noexcept {
    LogConvert<T>::Convert(m_message_, t);
    return std::move(*this);
  }

 private:
  const LoggerBase *m_logger_;
  time_t m_time_;
  std::string m_file_;
  std::uint16_t m_line_;
  LogLevel m_level_;
  pthread_t m_pid_;
  std::stringstream m_message_;
};

}  // namespace clsn

#endif  // DEFTRPC_LOGRECORD_H
