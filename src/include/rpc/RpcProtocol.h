//
// Created by lqf on 23-6-24.
//

#ifndef DEFTRPC_RPCPROTOCOL_H
#define DEFTRPC_RPCPROTOCOL_H

#include <string>
#include "common/Crc32.h"
#include "rpc/RpcException.h"
#include "serialize/StringSerializer.h"

namespace clsn {
struct RPCRequest {
  std::string m_func_name_;
  std::string m_args_;
  short m_async_;

  template <class Sr>
  void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr) {
    sr(m_func_name_, m_args_, m_async_);
  }

  template <class Sr>
  void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr) {
    sr(m_func_name_, m_args_, m_async_);
  }
};

struct RPCResponse {
  std::string m_res_;
  bool m_succeed_;

  template <class Sr>
  void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr) {
    sr(m_succeed_, m_res_);
  }

  template <class Sr>
  void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr) {
    sr(m_succeed_, m_res_);
  }

  template <class Res>
  Res GetResponseRes() {
    if (!clsn::CheckCec32(m_res_.data(), m_res_.size())) {
      throw std::logic_error(package_is_invalid);
    }
    Res r;
    clsn::StringSerialize decode(m_res_);
    m_res_(r);
    return r;
  }

  template <class Res>
  void GetResponseRes(Res &r) {
    if (!clsn::CheckCec32(m_res_.data(), m_res_.size())) {
      throw std::logic_error(package_is_invalid);
    }
    clsn::StringSerialize decode(m_res_);
    m_res_(r);
  }
};
}  // namespace clsn

#endif  // DEFTRPC_RPCPROTOCOL_H
