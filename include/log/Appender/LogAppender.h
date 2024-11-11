//
// Created by lqf on 23-3-30.
//

#ifndef DEFTRPC_LOGAPPENDER_H
#define DEFTRPC_LOGAPPENDER_H

#include <fstream>
#include <iostream>
#include <memory>
#include <vector>
#include "../LogCommon.h"
#include "../LogRecord.h"
#include "common/SpanMutex.h"
#include "common/common.h"

namespace CLSN {

class LogFormatter;

class LogAppender;

LogAppender *createConsoleLogAppender(const std::string &format, LogLevel level = LogLevel::Debug);

LogAppender *createFileLogAppender(const std::string &filename, const std::string &format,
                                   LogLevel level = LogLevel::Debug);

class LogAppender : noncopyable {
 public:
  explicit LogAppender(LogLevel level = LogLevel::Debug) noexcept;

  explicit LogAppender(const std::string &format, LogLevel level = LogLevel::Debug);

  virtual ~LogAppender();

  virtual void append(const LogRecord &) noexcept = 0;

  void setLogFormatter(const std::string &arg);

  void setLogFormatter(LogFormatter *IFormatter) noexcept;

  [[nodiscard]] virtual bool valid() const noexcept = 0;

  [[nodiscard]] bool recordValid(const LogRecord &record) const noexcept {
    return (static_cast<short>(record.getLevel()) >= static_cast<short>(appenderLevel)) && valid();
  }

 protected:
  CLSN::SpanMutex mutex;
  std::unique_ptr<LogFormatter> formatter;
  LogLevel appenderLevel;
};

class ConsoleLogAppender : public LogAppender {
 public:
  explicit ConsoleLogAppender(const std::string &format, LogLevel level = LogLevel::Debug)
      : LogAppender(format, level) {}

  explicit ConsoleLogAppender(LogLevel level = LogLevel::Debug) noexcept : LogAppender(level) {}

  ~ConsoleLogAppender() override = default;

  void append(const LogRecord &record) noexcept override;

  [[nodiscard]] bool valid() const noexcept override { return static_cast<bool>(formatter); }
};

class FileLogAppender : public LogAppender {
 public:
  FileLogAppender(const std::string &filename, const std::string &format, LogLevel level = LogLevel::Debug)
      : LogAppender(format, level) {
    reSetFileName(filename);
  }

  explicit FileLogAppender(const std::string &filename, LogLevel level = LogLevel::Debug) : LogAppender(level) {
    reSetFileName(filename);
  }

  explicit FileLogAppender(LogLevel level = LogLevel::Debug) noexcept : LogAppender(level) {}

  ~FileLogAppender() override {
    if (ofStream.is_open()) ofStream.close();
  }

  void reSetFileName(const std::string &filename) {
    if (ofStream.is_open()) {
      ofStream.close();
    }
    ofStream.open(filename, std::ofstream::out | std::ofstream::app);
    if (!ofStream.is_open()) {
      throw std::invalid_argument("filename is invalid");
    }
  }

  void append(const LogRecord &record) noexcept override;

  bool valid() const noexcept override { return ofStream.is_open() && static_cast<bool>(formatter); }

 private:
  std::ofstream ofStream;
};

}  // namespace CLSN

#endif  // DEFTRPC_LOGAPPENDER_H
