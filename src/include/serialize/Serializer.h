//
// Created by lqf on 23-4-24.
//

#ifndef DEFTRPC_SERIALIZER_H
#define DEFTRPC_SERIALIZER_H

#include <unistd.h>
#include <map>
#include <memory>
#include <utility>
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
  id_type RegisterPrt(const std::shared_ptr<T> ptr) {
    auto address = ptr.get();
    if (address == nullptr) {
      return 0;
    }
    auto it = m_address_to_id_.find(reinterpret_cast<address_type>(address));
    if (it == m_address_to_id_.end()) {
      ++m_cur_idx_;
      m_address_to_id_.emplace(reinterpret_cast<address_type>(address), m_cur_idx_);
      m_id_to_ptr_.emplace(m_cur_idx_, ptr);
      return m_cur_idx_ | PTR_ID_MASK;
    }
    return it->second;
  }

  std::shared_ptr<void> GetPrt(id_type id) {
    auto it = m_id_to_ptr_.find(id);
    if (m_id_to_ptr_.end() == it) {
      return nullptr;
    }
    return it->second;
  }

 private:
  std::map<address_type, id_type> m_address_to_id_;
  std::map<id_type, std::shared_ptr<void>> m_id_to_ptr_;
  address_type m_cur_idx_{0};
};
}  // namespace detail

template <class Drive>
class Serializer : public detail::SerializeBase {
 public:
  explicit Serializer(Drive *d) noexcept : SerializeBase(), m_self_(d) {}

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
    SerializeImpl(std::forward<Arg>(arg));
    return *this;
  }

 private:
  template <typename T>
  inline std::enable_if_t<!has_serialize_v<Drive, T>> SerializeImpl(T &&arg) {
    static_assert(!has_serialize_v<Drive, T>, "type t has no serialize m_func_");
  }

  template <typename T>
  inline std::enable_if_t<has_no_member_serialize_func_v<Drive, T> && !has_member_serialize_func_v<Drive, T>>
  SerializeImpl(T &&arg) {
    DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(*m_self_, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_member_serialize_func_v<Drive, T>> SerializeImpl(T &&arg) {
    access::DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(*m_self_, std::forward<T>(arg));
  }

 public:
  Drive *m_self_;
};

template <typename Drive>
class DeSerializer : public detail::SerializeBase {
 public:
  explicit DeSerializer(Drive *d) noexcept : SerializeBase(), m_self_(d) {}

  ~DeSerializer() override = default;

  template <typename Header, typename... Args>
  inline DeSerializer &operator()(Header &&header, Args &&...args) {
    operator()(std::forward<Header>(header));
    operator()(std::forward<Args>(args)...);
    return *this;
  }

  template <typename Arg>
  inline DeSerializer &operator()(Arg &&arg) {
    DeSerializeImpl(std::forward<Arg>(arg));
    return *this;
  }

 private:
  template <typename T>
  inline std::enable_if_t<!has_deserialize_v<Drive, T> && !has_load_and_construct_v<Drive, T>> DeSerializeImpl(
      T &&arg) {
    static_assert(has_deserialize_v<Drive, T> || has_load_and_construct_v<Drive, T>,
                  "type T has no deserialize m_func_");
  }

  template <typename T>
  inline std::enable_if_t<has_no_member_deserialize_func_v<Drive, T>> DeSerializeImpl(T &&arg) {
    DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(*m_self_, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_member_deserialize_func_v<Drive, T>> DeSerializeImpl(T &&arg) {
    access::DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(*m_self_, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_no_member_load_and_construct_func_v<Drive, T>> DeSerializeImpl(T &&arg) {
    DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(*m_self_, std::forward<T>(arg));
  }

  template <typename T>
  inline std::enable_if_t<has_member_load_and_construct_func_v<Drive, T>> DeSerializeImpl(T &&arg) {
    access::DEFTRPC_DESERIALIZE_LOAD_AND_CONSTRUCT_FUNCNANE(*m_self_, std::forward<T>(arg));
  }

 private:
  Drive *m_self_;
};
}  // namespace clsn

#endif  // DEFTRPC_SERIALIZER_H
