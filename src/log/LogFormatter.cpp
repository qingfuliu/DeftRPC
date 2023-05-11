//
// Created by lqf on 23-3-30.
//

#include "Formatter/LogFormatter.h"
#include  "LoggerBase.h"
#include  "Convert/LogConvert.h"
#include <cstring>
namespace CLSN {

    class TempFormatterItem : public FormatterItem {
    public:
        TempFormatterItem() noexcept: FormatterItem() {};

        void format(std::ostream &os, const LogRecord &) noexcept override {
        }
    };


    class TimeFormatterItem : public FormatterItem {
    public:
        explicit TimeFormatterItem(std::string &&str) noexcept: FormatterItem(), timeFormat(std::move(str)) {}

        ~TimeFormatterItem() override = default;

         void format(std::ostream &os, const LogRecord &record) noexcept override {
            struct tm tm{};
            time_t time = record.getTime();
            localtime_r(&time, &tm);
            char buf[64];
            strftime(buf, sizeof(buf), timeFormat.c_str(), &tm);
            os.rdbuf()->sputn(buf,static_cast<std::streamsize>(strlen(buf)));
        }

    private:
        std::string timeFormat;
    };

    class StrFormatterItem : public FormatterItem {
    public:
        explicit StrFormatterItem(std::string &&str) noexcept: FormatterItem(), v(std::move(str)) {}

        ~StrFormatterItem() override= default;

        void format(std::ostream &os, const LogRecord &) noexcept override {
            os.rdbuf()->sputn(v.c_str(),static_cast<std::streamsize>(v.size()));
        }

        void format(std::string &&temp) noexcept {
            v.append(temp);
        }

    private:
        std::string v;
    };


    class MessageFormatterItem : public FormatterItem {
    public:
        MessageFormatterItem() noexcept: FormatterItem() {}

        ~MessageFormatterItem()override = default;

         void format(std::ostream &os, const LogRecord &recode) noexcept override {
            os.rdbuf()->sputn(recode.getMessage().str().c_str(),
                              static_cast<std::streamsize>(recode.getMessage().str().size()));
        }
    };

    class LineFormatterItem : public FormatterItem {
    public:
        LineFormatterItem() noexcept: FormatterItem() {}

        ~LineFormatterItem()override = default;

         void format(std::ostream &os, const LogRecord &recode) noexcept override {
            LogConvert<unsigned short>::convert(os, recode.getLine());
        }
    };

    class FileNameFormatterItem : public FormatterItem {
    public:
        FileNameFormatterItem() noexcept: FormatterItem() {}

        ~FileNameFormatterItem()override = default;

         void format(std::ostream &os, const LogRecord &recode) noexcept override {
            os.rdbuf()->sputn(recode.getFile().c_str(),static_cast<std::streamsize>(recode.getFile().size()));
        }
    };

    class ThreadIdFormatterItem : public FormatterItem {
    public:
        ThreadIdFormatterItem() noexcept: FormatterItem() {}

        ~ThreadIdFormatterItem() override= default;

         void format(std::ostream &os, const LogRecord &recode) noexcept override {
            LogConvert<pthread_t>::convert(os, recode.getPid());
        }
    };

    class LogLevelNameFormatterItem : public FormatterItem {
    public:
        LogLevelNameFormatterItem() noexcept: FormatterItem() {}

        ~LogLevelNameFormatterItem()override = default;

         void format(std::ostream &os, const LogRecord &recode) noexcept override {
           std::string level=LogLevelToString(recode.getLevel());
            os.rdbuf()->sputn(level.c_str() ,static_cast<std::streamsize>(level.size()));
        }
    };

    class LoggerNameFormatterItem : public FormatterItem {
    public:
        LoggerNameFormatterItem() noexcept: FormatterItem() {}

        ~LoggerNameFormatterItem() override= default;

         void format(std::ostream &os, const LogRecord &recode) noexcept override {
            auto name=recode.getLogger()->getName();
            os.rdbuf()->sputn(name.c_str(),static_cast<std::streamsize>(name.size()));
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
    LogFormatter::LogFormatter(const std::string &format) : noncopyable(), first(new TempFormatterItem()) {
        size_t timeBegin = -1;
        size_t prev = 0;
        size_t index = format.find('%');
        for (; prev <= format.size() - 1 && index != std::string::npos;
               index = format.find('%', prev)) {

            do {
                if (index >= format.size() - 1) {
                    throw FormatError();
                } else if (format[index + 1] == '%') {
                    if (timeBegin != -1) {
                        prev = timeBegin;
                        timeBegin = -1;
                    }
                    first->addItem(
                            new StrFormatterItem(std::string(format.begin() + prev, format.begin() + index + 1))
                    );
                    break;
                }

                switch (format[index + 1]) {
#define APPEND_ITEM(CON, CLASSNAME, ARG) \
                case CON:             \
                if (prev < index) {\
                    if (timeBegin != -1) {\
                        prev = timeBegin;\
                        first->addItem(\
                                new TimeFormatterItem(std::string(format.begin() + prev, format.begin() + index))); \
                        timeBegin = -1;\
                    } else\
                        first->addItem(\
                                new StrFormatterItem(std::string(format.begin() + prev, format.begin() + index)));\
                }                        \
                first->addItem(        \
                new CLASSNAME(ARG));     \
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
                    APPEND_ITEM('n', LineFormatterItem,)
                    APPEND_ITEM('s', MessageFormatterItem,)
                    APPEND_ITEM('f', FileNameFormatterItem,)
                    APPEND_ITEM('t', ThreadIdFormatterItem,)
                    APPEND_ITEM('l', LogLevelNameFormatterItem,)
                    APPEND_ITEM('o', LoggerNameFormatterItem,)
#undef APPEND_ITEM
                    default:
                        throw FormatError();
                }
            } while (false);
            prev = index + 2;
        }

        if (timeBegin != -1) {
            first->addItem(
                    new TimeFormatterItem(std::string(format.begin() + timeBegin, format.end())));
        } else if (prev < format.size() - 1) {
            first->addItem(new StrFormatterItem(std::string(format.begin() + prev, format.end())));
        }

    }


    void LogFormatter::format(std::ostream &os, const LogRecord &record) noexcept {
        FormatterItem *temp = first;
        while (temp != nullptr) {
            temp->format(os, record);
            temp = temp->nextItem();
        }
        os<<std::endl << std::flush;
    }

} // CLSN