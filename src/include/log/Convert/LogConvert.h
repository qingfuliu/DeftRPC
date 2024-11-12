//
// Created by lqf on 23-3-30.
//

#ifndef DEFTRPC_LOGCONVERT_H
#define DEFTRPC_LOGCONVERT_H

#include <iostream>
#include <string>

#include <deque>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace clsn {

template <class T>
class LogConvert {
 public:
  static void convert(std::ostream &oss, const T &t) noexcept { oss << t; }
};

template <>
class LogConvert<std::string> {
 public:
  static void convert(std::ostream &oss, const std::string &t) noexcept { oss << t; }
};

#define CONVERTER_DEFINE(X)                                               \
  template <typename T>                                                   \
  class LogConvert<std::X<T>> {                                           \
   public:                                                                \
    static void convert(std::ostream &oss, const std::X<T> &t) noexcept { \
      oss << "<" #X ">[";                                                 \
      auto begin = t.cbegin();                                            \
      if (begin != t.cend()) {                                            \
        LogConvert<T>::convert(oss, *(begin++));                          \
        for (; begin != t.cend(); ++begin) {                              \
          oss << ",";                                                     \
          LogConvert<T>::convert(oss, *begin);                            \
        }                                                                 \
      }                                                                   \
      oss << "]";                                                         \
    }                                                                     \
  };

CONVERTER_DEFINE(vector)

CONVERTER_DEFINE(unordered_set)

CONVERTER_DEFINE(set)

CONVERTER_DEFINE(list)

CONVERTER_DEFINE(deque)

#undef CONVERTER_DEFINE

#define CONVERTER_MAP_DEFINE(X)                                                  \
  template <typename Key, typename Val>                                          \
  class LogConvert<std::X<Key, Val>> {                                           \
   public:                                                                       \
    static void convert(std::ostream &oss, const std::X<Key, Val> &t) noexcept { \
      oss << "<" #X ">[";                                                        \
      auto begin = t.cbegin();                                                   \
      if (begin != t.cend()) {                                                   \
        LogConvert<Key>::convert(oss, begin->pIovec);                            \
        oss << ":";                                                              \
        LogConvert<Val>::convert(oss, (begin++)->second);                        \
        for (; begin != t.cend(); ++begin) {                                     \
          oss << ",";                                                            \
          LogConvert<Key>::convert(oss, begin->pIovec);                          \
          oss << ":";                                                            \
          LogConvert<Val>::convert(oss, begin->second);                          \
        }                                                                        \
      }                                                                          \
      oss << "]";                                                                \
    }                                                                            \
  };

CONVERTER_MAP_DEFINE(map)

CONVERTER_MAP_DEFINE(unordered_map)

#undef CONVERTER_MAP_DEFINE
}  // namespace clsn

#endif  // DEFTRPC_LOGCONVERT_H
