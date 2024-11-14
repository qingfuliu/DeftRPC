//
// Created by lqf on 23-3-30.
//

#ifndef DEFTRPC_FORMATTER_H
#define DEFTRPC_FORMATTER_H

#include <iostream>
#include <string>
#include <vector>
#include "common/common.h"
#include "log/LogCommon.h"
#include "log/LogRecord.h"

namespace clsn {

class FormatterItem : public Noncopyable {
 public:
  virtual ~FormatterItem() = default;

  virtual void format(std::ostream &os, const LogRecord &) noexcept = 0;

  virtual FormatterItem *nextItem() noexcept { return next; }

  void addItem(FormatterItem *item) {
    if (item != nullptr) {
      auto temp = this;
      while (temp->next != nullptr) {
        temp = temp->next;
      }
      temp->next = item;
    }
  }

 private:
  FormatterItem *next = nullptr;
};

class LogFormatter : public Noncopyable {
 public:
  LogFormatter(const std::string &format);

  ~LogFormatter() {
    FormatterItem *temp;
    while (first != nullptr) {
      temp = first;
      first = first->nextItem();
      delete temp;
    }
  }

  void format(std::ostream &os, const LogRecord &) noexcept;

 private:
  FormatterItem *first;
};

}  // namespace clsn

#endif  // DEFTRPC_FORMATTER_H
