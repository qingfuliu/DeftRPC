//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_BINARYSERIALIZE_H
#define DEFTRPC_BINARYSERIALIZE_H

#include <iostream>
#include "Serializer.h"
#include "detail/helper.h"

namespace clsn {

class BinarySerialize : public Serializer<BinarySerialize> {
  using Base = Serializer<BinarySerialize>;

 public:
  explicit BinarySerialize(std::ostream &outPutStream) noexcept : Base(this), m_os_(outPutStream) {}

  ~BinarySerialize() override = default;

  void OutPut(const void *data, size_t size) {
    std::streamsize n = m_os_.rdbuf()->sputn(static_cast<const char *>(data), static_cast<std::streamsize>(size));
    if (size != n) {
      throw "";
    }
  }

 private:
  std::ostream &m_os_;
};

class FileBinarySerialize : public BinarySerialize {
  explicit FileBinarySerialize(const std::string &fileName) : BinarySerialize(m_of_) {
    m_of_.open(fileName, std::ios_base::trunc | std::ios_base::binary);
    if (!m_of_.is_open()) {
      throw "";
    }
  }

  ~FileBinarySerialize() override = default;

 private:
  std::ofstream m_of_;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void>>
void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(BinarySerialize &s, T &&a) {
  s.OutPut(reinterpret_cast<const char *>(std::addressof(a)), sizeof(T));
}

class BinaryDeSerialize : public DeSerializer<BinaryDeSerialize> {
  using Base = DeSerializer<BinaryDeSerialize>;

 public:
  explicit BinaryDeSerialize(std::istream &inPutStream) noexcept : Base(this), m_is_(inPutStream) {}

  ~BinaryDeSerialize() override = default;

  void Input(void *data, size_t size) {
    std::streamsize n = m_is_.rdbuf()->sgetn(static_cast<char *>(data), static_cast<std::streamsize>(size));
    if (size != n) {
      throw "";
    }
  }

 private:
  std::istream &m_is_;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void>>
void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(BinaryDeSerialize &s, T &&a) {
  s.Input(reinterpret_cast<char *>(std::addressof(a)), sizeof(T));
}

}  // namespace clsn

#endif  // DEFTRPC_BINARYSERIALIZE_H
