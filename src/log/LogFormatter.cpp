//
// Created by lqf on 23-3-30.
//

#include "log/Formatter/LogFormatter.h"
#include <cstring>
#include "log/Convert/LogConvert.h"
#include "log/LoggerBase.h"

namespace clsn {

class TempFormatterItem : public FormatterItem {
 public:
  TempFormatterItem() noexcept : FormatterItem(){};

  void Format(std::ostream &os, const LogRecord &record) noexcept override { (void)record; }
};

class TimeFormatterItem : public FormatterItem {
 public:
  explicit TimeFormatterItem(std::string &&str) noexcept : FormatterItem(), m_time_format_(std::move(str)) {}

  ~TimeFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &record) noexcept override {
    struct tm tm {};
    time_t time = record.GetTime();
    localtime_r(&time, &tm);
    char buf[64];
    strftime(buf, sizeof(buf), m_time_format_.c_str(), &tm);
    os.rdbuf()->sputn(buf, static_cast<std::streamsize>(strlen(buf)));
  }

 private:
  std::string m_time_format_;
};

class StrFormatterItem : public FormatterItem {
 public:
  explicit StrFormatterItem(std::string &&str) noexcept : FormatterItem(), m_v_(std::move(str)) {}

  ~StrFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &record) noexcept override {
    (void)record;
    os.rdbuf()->sputn(m_v_.c_str(), static_cast<std::streamsize>(m_v_.size()));
  }

  //        void format(std::string &&temp) noexcept { m_v_.append(temp); }

 private:
  std::string m_v_;
};

class MessageFormatterItem : public FormatterItem {
 public:
  MessageFormatterItem() noexcept : FormatterItem() {}

  ~MessageFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &recode) noexcept override {
    os.rdbuf()->sputn(recode.GetMessage().str().c_str(),
                      static_cast<std::streamsize>(recode.GetMessage().str().size()));
  }
};

class LineFormatterItem : public FormatterItem {
 public:
  LineFormatterItem() noexcept : FormatterItem() {}

  ~LineFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &recode) noexcept override {
    LogConvert<std::uint16_t>::Convert(os, recode.GetLine());
  }
};

class FileNameFormatterItem : public FormatterItem {
 public:
  FileNameFormatterItem() noexcept : FormatterItem() {}

  ~FileNameFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &recode) noexcept override {
    os.rdbuf()->sputn(recode.GetFile().c_str(), static_cast<std::streamsize>(recode.GetFile().size()));
  }
};

class ThreadIdFormatterItem : public FormatterItem {
 public:
  ThreadIdFormatterItem() noexcept : FormatterItem() {}

  ~ThreadIdFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &recode) noexcept override {
    LogConvert<pthread_t>::Convert(os, recode.GetPid());
  }
};

class LogLevelNameFormatterItem : public FormatterItem {
 public:
  LogLevelNameFormatterItem() noexcept : FormatterItem() {}

  ~LogLevelNameFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &recode) noexcept override {
    std::string level = LogLevelToString(recode.GetLevel());
    os.rdbuf()->sputn(level.c_str(), static_cast<std::streamsize>(level.size()));
  }
};

class LoggerNameFormatterItem : public FormatterItem {
 public:
  LoggerNameFormatterItem() noexcept : FormatterItem() {}

  ~LoggerNameFormatterItem() override = default;

  void Format(std::ostream &os, const LogRecord &recode) noexcept override {
    auto name = recode.GetLogger()->GetName();
    os.rdbuf()->sputn(name.c_str(), static_cast<std::streamsize>(name.size()));
  }
};

/* *
 *时间：%Y%m%d %H%M%S
 *行号：%n
 *文件名：%f
 *日志：%s
 *线程号：%t
 *日志等级：%l
 *日志器名字：%o
 */
LogFormatter::LogFormatter(const std::string &format) : m_first_(new TempFormatterItem()) {
  size_t timeBegin = -1;
  size_t prev = 0;
  size_t index = format.find('%');
  for (; prev <= format.size() - 1 && index != std::string::npos; index = format.find('%', prev)) {
    do {
      if (index >= format.size() - 1) {
        throw FormatError();
      }
      if (format[index + 1] == '%') {
        if (timeBegin != -1) {
          prev = timeBegin;
          timeBegin = -1;
        }
        m_first_->AddItem(new StrFormatterItem(std::string(format.begin() + prev, format.begin() + index + 1)));
        break;
      }

      switch (format[index + 1]) {
#define APPEND_ITEM(CON, CLASSNAME, ARG)                                                                      \
  case CON:                                                                                                   \
    if (prev < index) {                                                                                       \
      if (timeBegin != -1) {                                                                                  \
        prev = timeBegin;                                                                                     \
        m_first_->AddItem(new TimeFormatterItem(std::string(format.begin() + prev, format.begin() + index))); \
        timeBegin = -1;                                                                                       \
      } else                                                                                                  \
        m_first_->AddItem(new StrFormatterItem(std::string(format.begin() + prev, format.begin() + index)));  \
    }                                                                                                         \
    m_first_->AddItem(new CLASSNAME(ARG));                                                                    \
    break;
        case 'Y':
        case 'm':
        case 'd':
        case 'H':
        case 'M':
        case 'S':
          if (timeBegin == -1) {
            timeBegin = prev;
          }
          break;
          APPEND_ITEM('n', LineFormatterItem, )
          APPEND_ITEM('s', MessageFormatterItem, )
          APPEND_ITEM('f', FileNameFormatterItem, )
          APPEND_ITEM('t', ThreadIdFormatterItem, )
          APPEND_ITEM('l', LogLevelNameFormatterItem, )
          APPEND_ITEM('o', LoggerNameFormatterItem, )
#undef APPEND_ITEM
        default:
          throw FormatError();
      }
    } while (false);
    prev = index + 2;
  }

  if (timeBegin != -1) {
    m_first_->AddItem(new TimeFormatterItem(std::string(format.begin() + timeBegin, format.end())));
  } else if (prev < format.size() - 1) {
    m_first_->AddItem(new StrFormatterItem(std::string(format.begin() + prev, format.end())));
  }
}

void LogFormatter::Format(std::ostream &os, const LogRecord &record) noexcept {
  FormatterItem *temp = m_first_;
  while (temp != nullptr) {
    temp->Format(os, record);
    temp = temp->NextItem();
  }
  os << '\n' << std::flush;
}

}  // namespace clsn