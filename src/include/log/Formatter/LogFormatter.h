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
  ~FormatterItem() override = default;

  virtual void Format(std::ostream &os, const LogRecord &) noexcept = 0;

  virtual FormatterItem *NextItem() noexcept { return m_next_; }

  void AddItem(FormatterItem *item) {
    if (item != nullptr) {
      auto temp = this;
      while (temp->m_next_ != nullptr) {
        temp = temp->m_next_;
      }
      temp->m_next_ = item;
    }
  }

 private:
  FormatterItem *m_next_ = nullptr;
};

class LogFormatter : public Noncopyable {
 public:
  explicit LogFormatter(const std::string &format);

  ~LogFormatter() override {
    FormatterItem *temp;
    while (m_first_ != nullptr) {
      temp = m_first_;
      m_first_ = m_first_->NextItem();
      delete temp;
    }
  }

  void Format(std::ostream &os, const LogRecord &record) noexcept;

 private:
  FormatterItem *m_first_;
};

}  // namespace clsn

#endif  // DEFTRPC_FORMATTER_H
