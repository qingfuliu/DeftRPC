//
// Created by lqf on 23-3-30.
//

#include "Appender/LogAppender.h"
#include"Formatter/LogFormatter.h"
#include "common/mutex.h"

namespace CLSN {

    LogAppender *createConsoleLogAppender(const std::string &format, LogLevel level) {
        return new ConsoleLogAppender(format, level);
    }

    LogAppender *createFileLogAppender(const std::string &filename, const std::string &format,
                                       LogLevel level) {
        return new FileLogAppender(filename, format, level);
    }

    LogAppender::LogAppender(LogLevel level) noexcept:
            noncopyable(), formatter(nullptr), appenderLevel(level) {}

    LogAppender::LogAppender(const std::string &format, LogLevel level) :
            noncopyable(), formatter(new LogFormatter(format)), appenderLevel(level) {}

    LogAppender::~LogAppender()=default;

    void LogAppender::setLogFormatter(const std::string &arg) {
        formatter.reset(new LogFormatter(arg));
    }

    void LogAppender::setLogFormatter(LogFormatter *IFormatter) noexcept {
        formatter.reset(IFormatter);
    }


    void ConsoleLogAppender::append(const LogRecord &record) noexcept {
        CLSN::MutexGuard lock(&mutex);
        if (recordValid(record))
            formatter->format(std::cout, record);
    }

    void FileLogAppender::append(const LogRecord &record) noexcept {
        CLSN::MutexGuard lock(&mutex);
        if (recordValid(record))
            formatter->format(ofStream, record);
    }

//    ConsoleLogAppender::

} // CLSN