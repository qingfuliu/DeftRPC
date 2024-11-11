//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_BINARYSERIALIZE_H
#define DEFTRPC_BINARYSERIALIZE_H

#include <iostream>
#include "Serializer.h"
#include "detail/helper.h"

namespace CLSN {

class BinarySerialize : public Serializer<BinarySerialize> {
  using Base = Serializer<BinarySerialize>;

 public:
  explicit BinarySerialize(std::ostream &outPutStream) noexcept : Base(this), os(outPutStream) {}

  ~BinarySerialize() override = default;

  void OutPut(const void *data, size_t size) {
    std::streamsize n = os.rdbuf()->sputn(static_cast<const char *>(data), static_cast<std::streamsize>(size));
    if (size != n) {
      throw "";
    }
  }

 private:
  std::ostream &os;
};

class FileBinarySerialize : public BinarySerialize {
  explicit FileBinarySerialize(const std::string &fileName) : BinarySerialize(of) {
    of.open(fileName, std::ios_base::trunc | std::ios_base::binary);
    if (!of.is_open()) {
      throw "";
    }
  }

  ~FileBinarySerialize() override = default;

 private:
  std::ofstream of;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void>>
void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(BinarySerialize &s, T &&a) {
  s.OutPut(reinterpret_cast<const char *>(std::addressof(a)), sizeof(T));
}

class BinaryDeSerialize : public DeSerializer<BinaryDeSerialize> {
  using Base = DeSerializer<BinaryDeSerialize>;

 public:
  explicit BinaryDeSerialize(std::istream &inPutStream) noexcept : Base(this), is(inPutStream) {}

  ~BinaryDeSerialize() override = default;

  void Input(void *data, size_t size) {
    std::streamsize n = is.rdbuf()->sgetn(static_cast<char *>(data), static_cast<std::streamsize>(size));
    if (size != n) {
      throw "";
    }
  }

 private:
  std::istream &is;
};

template <typename T, typename = std::enable_if_t<std::is_arithmetic_v<std::remove_reference_t<T>>, void>>
void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(BinaryDeSerialize &s, T &&a) {
  s.Input(reinterpret_cast<char *>(std::addressof(a)), sizeof(T));
}

}  // namespace CLSN

#endif  // DEFTRPC_BINARYSERIALIZE_H
