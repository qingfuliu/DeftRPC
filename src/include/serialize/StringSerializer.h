//
// Created by lqf on 23-6-20.
//

#ifndef DEFTRPC_STRINGSERIALIZER_H
#define DEFTRPC_STRINGSERIALIZER_H

#include <exception>
#include <iostream>
#include <string>
#include "SerializeException.h"
#include "Serializer.h"
#include "detail/helper.h"

namespace clsn {

class StringSerialize : public Serializer<StringSerialize> {
  using Base = Serializer<StringSerialize>;

 public:
  explicit StringSerialize(std::string &str) noexcept : Base(this), m_str_(str) {}

  ~StringSerialize() override = default;

  void OutPut(const void *data, size_t size) noexcept {
    m_str_.append(static_cast<const char *>(data), static_cast<std::streamsize>(size));
  }

 private:
  std::string &m_str_;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void>>
void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(StringSerialize &s, T &&a) {
  s.OutPut(reinterpret_cast<const char *>(std::addressof(a)), sizeof(T));
}

class StringDeSerialize : public DeSerializer<StringDeSerialize> {
  using Base = DeSerializer<StringDeSerialize>;

 public:
  explicit StringDeSerialize(std::string_view inPutStr) noexcept : Base(this), m_str_(inPutStr) {}

  ~StringDeSerialize() override = default;

  void Input(void *data, size_t size) {
    if (m_str_.size() < size) {
      throw std::logic_error(args_length_error);
    }
    m_str_.copy(static_cast<char *>(data), size);
    m_str_ = m_str_.substr(size);
  }

 private:
  std::string_view m_str_;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void>>
void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(StringDeSerialize &s, T &&a) {
  s.Input(reinterpret_cast<char *>(std::addressof(a)), sizeof(T));
}

}  // namespace clsn

#endif  // DEFTRPC_STRINGSERIALIZER_H
