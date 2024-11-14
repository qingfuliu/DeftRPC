//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_SERIALIZER_H
#define DEFTRPC_SERIALIZER_H

#include <unistd.h>
#include <map>
#include <memory>
#include "detail/access.h"
#include "detail/config.h"
#include "detail/helper.h"
#include "type/common.h"

namespace clsn {

namespace detail {

class SerializeBase {
 public:
  SerializeBase() = default;

  virtual ~SerializeBase() = default;

  template <class T>
  IdType RegisterPrt(const std::shared_ptr<T> ptr) {
    auto address = ptr.get();
    if (address == nullptr) {
      return 0;
    }
    auto it = addressToId.find(reinterpret_cast<AddressType>(address));
    if (it == addressToId.end()) {
      ++curIdx;
      addressToId.emplace(reinterpret_cast<AddressType>(address), curIdx);
      IdToPtr.emplace(curIdx, ptr);
      return curIdx | PtrIdMask;
    }
    return it->second;
  }

  std::shared_ptr<void> GetPrt(IdType id) {
    auto it = IdToPtr.find(id);
    if (IdToPtr.end() == it) {
      return nullptr;
    }
    return it->second;
  }

 private:
  std::map<AddressType, IdType> addressToId;
  std::map<IdType, std::shared_ptr<void>> IdToPtr;
  AddressType curIdx{0};
};
}  // namespace detail

template <class Drive>
class Serializer : public detail::SerializeBase {
 public:
  explicit Serializer(Drive *d) noexcept : SerializeBase(), self(d) {}

  ~Serializer() override = default;

 public:
  template <typename Header, typename... Args>
  inline Serializer &operator()(Header &&header, Args &&...args) {
    operator()(std::forward<Header>(header));
    operator()(std::forward<Args>(args)...);
    return *this;
  }

  template <typename Arg>
  inline Serializer &operator()(Arg &&arg) {
    serializeImpl(std::forward<Arg>(arg));
    return *this;
  }

 private:
  template <typename T>
  inline std::enable_if_t<!has_serialize_v<Drive, T>> serializeImpl(T &&arg) {
    static_assert(!has_serialize_v<Drive, T>, "type t has no serialize m_func_");
  }

  template <typename T>
  inline std::enable_if_t<has_no_member_serialize_func_v<Drive, T> && !has_member_serialize_func_v<Drive, T>>
  serializeImpl(T &&arg) {
    DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(*self, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_member_serialize_func_v<Drive, T>> serializeImpl(T &&arg) {
    access::DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(*self, std::forward<T>(arg));
  }

 public:
  Drive *self;
};

template <typename Drive>
class DeSerializer : public detail::SerializeBase {
 public:
  explicit DeSerializer(Drive *d) noexcept : SerializeBase(), self(d) {}

  ~DeSerializer() override = default;

  template <typename Header, typename... Args>
  inline DeSerializer &operator()(Header &&header, Args &&...args) {
    operator()(std::forward<Header>(header));
    operator()(std::forward<Args>(args)...);
    return *this;
  }

  template <typename Arg>
  inline DeSerializer &operator()(Arg &&arg) {
    deSerializeImpl(std::forward<Arg>(arg));
    return *this;
  }

 private:
  template <typename T>
  inline std::enable_if_t<!has_deserialize_v<Drive, T> && !has_load_and_construct_v<Drive, T>> deSerializeImpl(
      T &&arg) {
    static_assert(has_deserialize_v<Drive, T> || has_load_and_construct_v<Drive, T>, "type T has no deserialize m_func_");
  }

  template <typename T>
  inline std::enable_if_t<has_no_member_deserialize_func_v<Drive, T>> deSerializeImpl(T &&arg) {
    DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(*self, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_member_deserialize_func_v<Drive, T>> deSerializeImpl(T &&arg) {
    access::DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(*self, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_no_member_load_and_construct_func_v<Drive, T>> deSerializeImpl(T &&arg) {
    DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(*self, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_member_load_and_construct_func_v<Drive, T>> deSerializeImpl(T &&arg) {
    access::DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(*self, std::forward<T>(arg));
  }

 private:
  Drive *self;
};
}  // namespace clsn

#endif  // DEFTRPC_SERIALIZER_H
