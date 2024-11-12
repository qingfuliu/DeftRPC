//
// Created by lqf on 23-6-22.
//
#include "rpc/RpcFunctional.h"
#include "serialize/StringSerializer.h"

namespace clsn::detail {

//    template<class Func>
//    template<typename T, size_t...index>
//    auto FunctionHelper<Func>::InterExecute(T &&tuple, Sequence<index...>) -> typename FuncTraitType::ResType {
//        return f(std::get<index>(tuple)...);
//    }

//    template<class Func>
//    std::string FunctionHelper<Func>::Call(std::string_view arg) {
//        std::string resStr;
//        std::exception_ptr eptr;
//        typename FuncTraitType::Res res;
//
//        clsn::StringDeSerialize decoder(arg);
//        typename FuncTraitType::TupleType tuple;
//
//        try {
//            decoder(tuple);
//            res = InterExecute(tuple, typename FuncTraitType::IndexSequenceType{});
//        } catch (...) {
//            eptr = std::current_exception();
//        }
//
//        if (eptr != nullptr) {
//            HandleException(resStr, eptr);
//            return resStr;
//        }
//
//        clsn::StringSerialize encoder(resStr);
//        encoder(res);
//        return resStr;
//    }
}  // namespace clsn::detail