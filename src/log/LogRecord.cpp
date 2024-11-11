#include "LogRecord.h"

namespace CLSN {
LogRecord::LogRecord(const LoggerBase *logger_, time_t time_, std::string &&file_, unsigned short line_,
                     LogLevel level_, pthread_t pid_) noexcept
    : logger(logger_), time(time_), file(std::move(file_)), line(line_), level(level_), pid(pid_) {}
}  // namespace CLSN