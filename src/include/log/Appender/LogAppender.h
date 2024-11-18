//
// Created by lqf on 23-3-30.
//

#ifndef DEFTRPC_LOGAPPENDER_H
#define DEFTRPC_LOGAPPENDER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "../LogCommon.h"
#include "../LogRecord.h"
#include "common/SpanMutex.h"
#include "common/common.h"

namespace clsn {

class LogFormatter;

class LogAppender;

LogAppender *CreateConsoleLogAppender(const std::string &format, LogLevel level = LogLevel::Debug);

LogAppender *CreateFileLogAppender(const std::string &filename, const std::string &format,
                                   LogLevel level = LogLevel::Debug);

class LogAppender : Noncopyable {
 public:
  explicit LogAppender(LogLevel level = LogLevel::Debug) noexcept;

  explicit LogAppender(const std::string &format, LogLevel level = LogLevel::Debug);

  ~LogAppender() override;

  virtual void Append(const LogRecord &) noexcept = 0;

  void SetLogFormatter(const std::string &arg);

  void SetLogFormatter(LogFormatter *IFormatter) noexcept;

  [[nodiscard]] virtual bool Valid() const noexcept = 0;

  [[nodiscard]] bool RecordValid(const LogRecord &record) const noexcept {
    return (static_cast<std::int16_t>(record.GetLevel()) >= static_cast<std::int16_t>(m_appender_level_)) && Valid();
  }

 protected:
  clsn::SpanMutex m_mutex_;
  std::unique_ptr<LogFormatter> m_formatter_;
  LogLevel m_appender_level_;
};

class ConsoleLogAppender : public LogAppender {
 public:
  explicit ConsoleLogAppender(const std::string &format, LogLevel level = LogLevel::Debug)
      : LogAppender(format, level) {}

  explicit ConsoleLogAppender(LogLevel level = LogLevel::Debug) noexcept : LogAppender(level) {}

  ~ConsoleLogAppender() override = default;

  void Append(const LogRecord &record) noexcept override;

  [[nodiscard]] bool Valid() const noexcept override { return static_cast<bool>(m_formatter_); }
};

class FileLogAppender : public LogAppender {
 public:
  FileLogAppender(const std::string &filename, const std::string &format, LogLevel level = LogLevel::Debug)
      : LogAppender(format, level) {
    ReSetFileName(filename);
  }

  explicit FileLogAppender(const std::string &filename, LogLevel level = LogLevel::Debug) : LogAppender(level) {
    ReSetFileName(filename);
  }

  explicit FileLogAppender(LogLevel level = LogLevel::Debug) noexcept : LogAppender(level) {}

  ~FileLogAppender() override {
    if (m_of_stream_.is_open()) {
      m_of_stream_.close();
    }
  }

  void ReSetFileName(const std::string &filename) {
    if (m_of_stream_.is_open()) {
      m_of_stream_.close();
    }
    m_of_stream_.open(filename, std::ofstream::out | std::ofstream::app);
    if (!m_of_stream_.is_open()) {
      throw std::invalid_argument("filename is invalid");
    }
  }

  void Append(const LogRecord &record) noexcept override;

  bool Valid() const noexcept override { return m_of_stream_.is_open() && static_cast<bool>(m_formatter_); }

 private:
  std::ofstream m_of_stream_;
};

}  // namespace clsn

#endif  // DEFTRPC_LOGAPPENDER_H
