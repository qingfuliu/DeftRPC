//
// Created by lqf on 23-3-30.
//

#ifndef DEFTRPC_LOGGERBASE_H
#define DEFTRPC_LOGGERBASE_H

#include <vector>
#include "Appender/LogAppender.h"
#include "LogCommon.h"
#include "LogRecord.h"
#include "common/common.h"

namespace clsn {

class LoggerBase {
  SINGLETON_BASE_DEFINE(LoggerBase)

 public:
  const std::string &GetName() const noexcept { return m_name_; }

  void SetName(const std::string &name) noexcept { this->m_name_ = name; }

  LogLevel GetLevel() const noexcept { return m_level_; }

  void SetLevel(LogLevel level) noexcept { this->m_level_ = level; }

  void AddLogAppender(LogAppender *appender) noexcept {
    m_appenders_.resize(m_appenders_.size() + 1);
    m_appenders_.emplace_back(appender);
  }

  void AddLogAppender(const std::vector<LogAppender *> &appender) noexcept {
    for (auto v : appender) {
      m_appenders_.emplace_back(v);
    }
    m_appenders_.shrink_to_fit();
  }

  void DoLog(LogRecord &&record) noexcept {
    for (const auto &appender : m_appenders_) {
      appender->Append(record);
    }
  }

  LoggerBase &operator+=(LogRecord &&record) noexcept {
    DoLog(std::move(record));
    return *this;
  }

 private:
  std::string m_name_{};
  LogLevel m_level_{};
  std::vector<std::unique_ptr<LogAppender>> m_appenders_;
};

template <std::uint16_t id = 0>
class Logger : public Singleton<Logger<id>>, public LoggerBase {
  SINGLETON_DEFINE(Logger)
};

template <std::uint16_t id = 0>
inline Logger<id> &getLogger() {
  return Logger<id>::GetInstance();
}

template <std::uint16_t id = 0>
inline void Init(const std::vector<LogAppender *> &appenders) {
  getLogger<id>().AddLogAppender(appenders);
}

}  // namespace clsn

#endif  // DEFTRPC_LOGGERBASE_H
