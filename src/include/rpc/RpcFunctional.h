//
// Created by lqf on 23-6-20.
//

#ifndef DEFTRPC_RPCFUNCTIONAL_H
#define DEFTRPC_RPCFUNCTIONAL_H

#include <sstream>
#include <tuple>
#include "common/Exception.h"
#include "rpc/RpcFunctional.h"
#include "serialize/StringSerializer.h"

namespace clsn {

class RpcFunction {
 public:
  RpcFunction() noexcept = default;

  template <class Func, class... Args>
  explicit RpcFunction(Func &&f, Args &&...args) noexcept {}

  virtual ~RpcFunction() = default;

  virtual std::string Call(std::string_view arg) = 0;

 private:
};

namespace detail {

template <size_t... index>
class Sequence {
 public:
  using next = Sequence<index..., sizeof...(index)>;
};

template <size_t index>
class IndexSequence {
 public:
  using type = typename IndexSequence<index - 1>::next;
  using next = typename type::next;
};

template <>
class IndexSequence<0> {
 public:
  using type = Sequence<0>;
  using next = type::next;
};

template <class Func>
class FuncTrait;

template <class Res, class... Args>
class FuncTrait<Res (*)(Args...)> {
 public:
  using ResType = Res;
  using TupleType = std::tuple<Args...>;
  using ArgsSize = std::integral_constant<size_t, sizeof...(Args)>;
  using IndexSequenceType = typename IndexSequence<ArgsSize::value - 1>::type;
};

template <class Func>
class FunctionHelper : public RpcFunction {
  using FuncTraitType = FuncTrait<Func>;

  template <typename T, size_t... index>
  auto InterExecute(T &&tuple, Sequence<index...>) -> typename FuncTraitType::ResType {
    return f(std::get<index>(tuple)...);
  }

 public:
  explicit FunctionHelper(Func &&f) noexcept : f(std::forward<Func>(f)) {}

  std::string Call(std::string_view arg) override {
    std::string resStr;
    std::exception_ptr eptr;
    typename FuncTraitType::ResType res;

    clsn::StringDeSerialize decoder(arg);
    typename FuncTraitType::TupleType tuple;

    try {
      decoder(tuple);
      res = InterExecute(tuple, typename FuncTraitType::IndexSequenceType{});
    } catch (...) {
      eptr = std::current_exception();
    }

    if (eptr != nullptr) {
      HandleException(resStr, eptr);
      return resStr;
    }

    clsn::StringSerialize encoder(resStr);
    encoder(res);
    return resStr;
  }

 private:
  Func f;
};
}  // namespace detail

//    template<class Func, class... Args>
//    inline RpcFunction *MakeRpcFunc(Func f, Args &&...args) noexcept {
//        return new detail::FunctionHelper(std::forward<Func>(f), std::forward<Args>(args)...);
//    }

template <class Func, class... Args>
inline RpcFunction *MakeRpcFunc(Func &&f) noexcept {
  return static_cast<RpcFunction *>(new detail::FunctionHelper(std::forward<Func>(f)));
}

}  // namespace clsn

#endif  // DEFTRPC_RPCFUNCTIONAL_H
