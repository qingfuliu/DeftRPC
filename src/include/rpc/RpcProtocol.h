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
  std::string funcName;
  std::string args;
  short async;

  template <class Sr>
  void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr) {
    sr(funcName, args, async);
  }

  template <class Sr>
  void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr) {
    sr(funcName, args, async);
  }
};

struct RPCResponse {
  std::string res;
  bool succeed;

  template <class Sr>
  void DEFTRPC_SERIALIZE_OUTPUT_FUNCNAME(Sr &sr) {
    sr(succeed, res);
  }

  template <class Sr>
  void DEFTRPC_DESERIALIZE_INPUT_FUNCNAME(Sr &sr) {
    sr(succeed, res);
  }

  template <class Res>
  Res GetResponseRes() {
    if (!clsn::CheckCec32(res.data(), res.size())) {
      throw std::logic_error(PackageIsInvalid);
    }
    Res r;
    clsn::StringSerialize decode(res);
    res(r);
    return r;
  }

  template <class Res>
  void GetResponseRes(Res &r) {
    if (!clsn::CheckCec32(res.data(), res.size())) {
      throw std::logic_error(PackageIsInvalid);
    }
    clsn::StringSerialize decode(res);
    res(r);
  }
};
}  // namespace clsn

#endif  // DEFTRPC_RPCPROTOCOL_H
