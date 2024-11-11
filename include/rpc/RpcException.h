//
// Created by lqf on 23-6-20.
//

#ifndef DEFTRPC_RPCEXCEPTION_H
#define DEFTRPC_RPCEXCEPTION_H

#include <string>

namespace CLSN {

inline const char *NoSuchFunction = "No Such Function.";
inline const char *FuncAlreadyExists = "Func Already Exists.";
inline const char *RouterIsInvalid = "Router Is Invalid.";
inline const char *PackageIsInvalid = "Package is Invalid.";
inline const char *SeverIsBase = "Sever is base or Connection is invalid.";

inline std::string MakeRpcException(const char *eptr, const std::string &funcName) {
  return std::string(eptr).append("The FuncName is ").append(funcName);
}
}  // namespace CLSN
#endif  // DEFTRPC_RPCEXCEPTION_H
