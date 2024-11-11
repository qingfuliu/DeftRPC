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

namespace CLSN {

class LoggerBase {
  SINGLETON_BASE_DEFINE(LoggerBase)

 public:
  const std::string &getName() const noexcept { return name; }

  void setName(const std::string &name) noexcept { this->name = name; }

  LogLevel getLevel() const noexcept { return level; }

  void setLevel(LogLevel level) noexcept { this->level = level; }

  void addLogAppender(LogAppender *appender) noexcept {
    appenders.resize(appenders.size() + 1);
    appenders.emplace_back(appender);
  }

  void addLogAppender(const std::vector<LogAppender *> &appender) noexcept {
    for (auto v : appender) {
      appenders.emplace_back(v);
    }
    appenders.shrink_to_fit();
  }

  void doLog(LogRecord &&record) noexcept {
    for (const auto &appender : appenders) {
      appender->append(record);
    }
  }

  LoggerBase &operator+=(LogRecord &&record) noexcept {
    doLog(std::move(record));
    return *this;
  }

 private:
  std::string name{};
  LogLevel level{};
  std::vector<std::unique_ptr<LogAppender>> appenders;
};

template <unsigned short Id = 0>
class Logger : public Singleton<Logger<Id>>, public LoggerBase {
  SINGLETON_DEFINE(Logger)
};

template <unsigned short Id = 0>
inline Logger<Id> &getLogger() {
  return Logger<Id>::getInstance();
}

template <unsigned short Id = 0>
inline void init(const std::vector<LogAppender *> &appenders) {
  getLogger<Id>().addLogAppender(appenders);
}

}  // namespace CLSN

#endif  // DEFTRPC_LOGGERBASE_H
