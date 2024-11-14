//
// Created by lqf on 23-3-30.
//

#include "log/Appender/LogAppender.h"
#include "common/mutex.h"
#include "log/Formatter/LogFormatter.h"

namespace clsn {

LogAppender *CreateConsoleLogAppender(const std::string &format, LogLevel level) {
  return new ConsoleLogAppender(format, level);
}

LogAppender *CreateFileLogAppender(const std::string &filename, const std::string &format, LogLevel level) {
  return new FileLogAppender(filename, format, level);
}

LogAppender::LogAppender(LogLevel level) noexcept : Noncopyable(), m_formatter_(nullptr), m_appender_level_(level) {}

LogAppender::LogAppender(const std::string &format, LogLevel level)
    : Noncopyable(), m_formatter_(new LogFormatter(format)), m_appender_level_(level) {}

LogAppender::~LogAppender() = default;

void LogAppender::SetLogFormatter(const std::string &arg) { m_formatter_.reset(new LogFormatter(arg)); }

void LogAppender::SetLogFormatter(LogFormatter *IFormatter) noexcept { m_formatter_.reset(IFormatter); }

void ConsoleLogAppender::Append(const LogRecord &record) noexcept {
  clsn::MutexGuard lock(&m_mutex_);
  if (RecordValid(record)) m_formatter_->format(std::cout, record);
}

void FileLogAppender::Append(const LogRecord &record) noexcept {
  clsn::MutexGuard lock(&m_mutex_);
  if (RecordValid(record)) m_formatter_->format(m_of_stream_, record);
}

//    ConsoleLogAppender::

}  // namespace clsn