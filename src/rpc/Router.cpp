//
// Created by lqf on 23-6-20.
//

#include "rpc/Router.h"

namespace clsn {

std::string RpcRouter::CallFuncSync(const std::string &funcName, std::string_view arg) {
  std::string res;
  std::exception_ptr eptr;
  if (auto func = m_functions_.FindByKey(funcName); func != nullptr) {
    auto rpc_function = *static_cast<RpcFunction **>(func->GetVal());
    try {
      res = rpc_function->Call(arg);
    } catch (...) {
      CLSN_LOG_FATAL << "An Exception happened.";
      eptr = std::current_exception();
    }

  } else {
    CLSN_LOG_DEBUG << MakeRpcException(no_such_function, funcName);
    eptr = std::make_exception_ptr(clsn::RpcExecuteException(MakeRpcException(no_such_function, funcName)));
  }

  HandleException(res, eptr);
  return res;
}

void RpcRouter::CallFuncASync(const std::string &funcName, std::string_view arg) {}
}  // namespace clsn