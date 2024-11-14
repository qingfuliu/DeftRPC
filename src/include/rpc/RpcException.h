//
// Created by lqf on 23-6-20.
//

#ifndef DEFTRPC_RPCEXCEPTION_H
#define DEFTRPC_RPCEXCEPTION_H

#include <string>

namespace clsn {

inline const char *no_such_function = "No Such Function.";
inline const char *func_already_exists = "Func Already Exists.";
inline const char *router_is_invalid = "Router Is Invalid.";
inline const char *package_is_invalid = "Package is Invalid.";
inline const char *sever_is_base = "Sever is base or Connection is invalid.";

inline std::string MakeRpcException(const char *eptr, const std::string &funcName) {
  return std::string(eptr).append("The FuncName is ").append(funcName);
}
}  // namespace clsn
#endif  // DEFTRPC_RPCEXCEPTION_H
