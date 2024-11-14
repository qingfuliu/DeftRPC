//
// Created by lqf on 23-4-28.
//

#ifndef DEFTRPC_ROUTER_H
#define DEFTRPC_ROUTER_H

#include <exception>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include "RpcException.h"
#include "RpcFunctional.h"
#include "common/Exception.h"
#include "common/Iterator.h"
#include "dataStruct/Hash.h"
#include "log/Log.h"

namespace clsn {
class RpcRouter {
 public:
  explicit RpcRouter(std::string name = "router") noexcept : routerName(std::move(name)) {}

  ~RpcRouter() = default;

  template <class Func, class... Args>
  void InsertFunc(const std::string &funcName, Func f, Args &&...args) {
    if (auto p = functions.Insert(funcName, MakeRpcFunc(std::forward<Func>(f), std::forward<Args>(args)...));
        nullptr == p) {
      CLSN_LOG_FATAL << MakeRpcException(func_already_exists, funcName);
      throw clsn::RpcExecuteException(MakeRpcException(func_already_exists, funcName));
    }
  }

  void DeleteFunc(const std::string &funcName) noexcept { functions.Delete(funcName); }

  std::string CallFuncSync(const std::string &funcName, std::string_view arg);

  void CallFuncASync(const std::string &funcName, std::string_view arg);

  std::unique_ptr<Iterator> GetIterator() noexcept { return functions.GetIterator(); }

 private:
  std::string routerName;
  HashTable functions;
};
}  // namespace clsn

#endif  // DEFTRPC_ROUTER_H
