//
// Created by lqf on 23-6-20.
//

#include "rpc/Router.h"

namespace CLSN {

std::string RpcRouter::CallFuncSync(const std::string &funcName, std::string_view arg) {
  std::string res;
  std::exception_ptr eptr;
  if (auto func = functions.FindByKey(funcName); func != nullptr) {
    auto rpcFunction = *static_cast<RpcFunction **>(func->GetVal());
    try {
      res = rpcFunction->Call(arg);
    } catch (...) {
      CLSN_LOG_FATAL << "An Exception happened.";
      eptr = std::current_exception();
    }

  } else {
    CLSN_LOG_DEBUG << MakeRpcException(NoSuchFunction, funcName);
    eptr = std::make_exception_ptr(CLSN::RpcExecuteException(MakeRpcException(NoSuchFunction, funcName)));
  }

  HandleException(res, eptr);
  return res;
}

void RpcRouter::CallFuncASync(const std::string &funcName, std::string_view arg) {}
}  // namespace CLSN