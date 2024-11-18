#include "log/LogRecord.h"

namespace clsn {
LogRecord::LogRecord(const LoggerBase *logger_, time_t time_, std::string &&file_, std::uint16_t line_, LogLevel level_,
                     pthread_t pid_) noexcept
    : m_logger_(logger_), m_time_(time_), m_file_(std::move(file_)), m_line_(line_), m_level_(level_), m_pid_(pid_) {}
}  // namespace clsn
