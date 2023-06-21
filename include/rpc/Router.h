//
// Created by lqf on 23-4-28.
//

#ifndef DEFTRPC_ROUTER_H
#define DEFTRPC_ROUTER_H

#include<unordered_map>
#include <string_view>
#include <exception>
#include <string>
#include<map>
#include "RpcException.h"
#include "RpcFunctional.h"
#include "dataStruct/Hash.h"
#include "common/Exception.h"

namespace CLSN {
    class RpcRouter {
    public:
        explicit RpcRouter(std::string name) noexcept:
                routerName(std::move(name)) {

        }

        template<class Func, class... Args>
        void InsertFunc(const std::string &funcName, Func f, Args &&...args) {
            if (auto p = functions.Insert(funcName, MakeRpcFunc(std::forward<Func>(f), std::forward<Args>(args)...));
                    nullptr == p) {
                throw CLSN::RpcExecuteException(MakeRpcException(FuncAlreadyExists, funcName));
            }
        }


//        std::string CallFuncAsync(std::string_view arg) {
//            return "";
//        }

        std::string CallFuncSync(const std::string &funcName, std::string_view arg);

    private:
        std::string routerName;
        HashTable functions;
    };
}


#endif //DEFTRPC_ROUTER_H
