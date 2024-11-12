#ifndef DEFTRPC_LOGRCOMMEN_H
#define DEFTRPC_LOGRCOMMEN_H

#include <memory>
#include <mutex>
#include <stdexcept>

namespace clsn {

using LevelType = unsigned short;

enum class LogLevel : unsigned short { None = 0, Debug = 1, Warning = 2, Error = 3, Fatal = 4 };

inline std::string LogLevelToString(LogLevel level) noexcept {
  switch (level) {
#define LEVELTOSTRING(X) \
  case LogLevel::X:      \
    return #X;

    LEVELTOSTRING(None)
    LEVELTOSTRING(Debug)
    LEVELTOSTRING(Warning)
    LEVELTOSTRING(Error)
    LEVELTOSTRING(Fatal)
#undef LEVELTOSTRING
    default:
      break;
  }
  return "None";
}

inline LogLevel StringToLogLevel(const std::string &level) noexcept {
  if (level.size() <= 0) {
    return LogLevel::None;
  }
  switch (level[0]) {
#define LEVELFromSTRING(X, T) \
  case X:                     \
    return T;

    LEVELFromSTRING('N', LogLevel::None) LEVELFromSTRING('D', LogLevel::Debug) LEVELFromSTRING('W', LogLevel::Warning)
        LEVELFromSTRING('E', LogLevel::Error) LEVELFromSTRING('F', LogLevel::Fatal) default : return LogLevel::None;
#undef LEVELFromSTRING
  }
}

class FormatError : public std::invalid_argument {
 public:
  FormatError() : std::invalid_argument("The format of the argument is incorrect") {}

  ~FormatError() = default;
};

}  // namespace clsn

#endif