//
// Created by lqf on 23-6-24.
//

#ifndef DEFTRPC_RPCCLIENT_H
#define DEFTRPC_RPCCLIENT_H

#include <future>
#include <string>
#include <thread>
#include <utility>
#include "net/client/TcpClient.h"
#include "rpc/RpcProtocol.h"
#include "serialize/BinarySerialize.h"
#include "serialize/StringSerializer.h"

namespace clsn {

class RpcClient : public TcpClient {
 public:
  explicit RpcClient(const std::string &ipPort) noexcept;

  template <class Res, class... Args>
  Res DoFuncSync(const std::string &name, Args &&...args) {
    DoRpcFunc(name, std::forward<Args>(args)...);
  }

  template <class Res, class... Args>
  std::future<Res> DoFuncAsync(std::string &&name, Args &&...args) {
    auto future = std::async([=]() { return this->DoRpcFunc<Res>(name, args...); });
    return future;
  }

  template <typename Clock, typename Dur, class Res, class... Args>
  Res DoFuncAsyncUntil(std::chrono::time_point<Clock, Dur> point, std::string &&name, Args &&...args) {
    auto future =
        std::async([=, name = std::move(name)]() { return this->DoRpcFunc<Res>(name, std::forward<Args>(args)...); });
    if (auto state = future.wait_until(point);
        state == std::future_status::timeout || state == std::future_status::deferred) {
      return {};
    }
    return future.Get();
  }

  template <typename Res, typename Rep, class Period, class... Args>
  Res DoFuncAsyncFor(std::chrono::duration<Rep, Period> point, std::string &&name, Args &&...args) {
    return {};
  }

 private:
  template <class Res, class... Args>
  Res DoRpcFunc(const std::string &name, Args &&...args) {
    //=================== StringSerialize ===================//
    clsn::RPCRequest request;
    request.m_func_name_ = name;
    request.m_async_ = static_cast<std::int16_t>(clsn::kRpcType::Sync);
    {
      clsn::StringSerialize encode(request.m_args_);
      encode(std::forward_as_tuple(args...));
    }

    {
      std::string data;
      clsn::StringSerialize encode(data);
      encode(request);
      //=================== send data ===================//
      if (-1 == TcpClient::Send(data)) {
        throw std::logic_error(sever_is_base);
      }
    }

    std::string_view view = TcpClient::Receive();
    clsn::RPCResponse response;
    {
      clsn::StringDeSerialize decode(view);
      decode(response);
    }
    clsn::StringDeSerialize decode(response.m_res_);
    if (response.m_succeed_) {
      Res res;
      decode(res);
      return res;
    }

    throw std::logic_error(response.m_res_);
  }

 private:
  int m_a_{};
};

}  // namespace clsn

#endif  // DEFTRPC_RPCCLIENT_H
