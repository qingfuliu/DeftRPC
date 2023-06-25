//
// Created by lqf on 23-6-24.
//

#ifndef DEFTRPC_RPCCLIENT_H
#define DEFTRPC_RPCCLIENT_H

#include <string>
#include <thread>
#include <future>
#include "rpc/RpcProtocol.h"
#include "net/client/TcpClient.h"
#include "serialize/BinarySerialize.h"
#include "serialize/StringSerializer.h"

namespace CLSN {

    class RpcClient : public TcpClient {
    public:
        explicit RpcClient(const std::string &ipPort) noexcept;

        template<class Res, class ...Args>
        Res DoFuncSync(const std::string &name, Args &&...args) {
            DoRpcFunc(name, std::forward<Args>(args)...);
        }

        template<class Res, class ...Args>
        std::future<Res> DoFuncAsync(std::string &&name, Args &&...args) {
            auto future = std::async([=]() {
                return this->DoRpcFunc<Res>(name, args...);
            });
            return future;
        }

        template<typename Clock, typename Dur, class Res, class ...Args>
        Res DoFuncAsyncUntil(std::chrono::time_point<Clock, Dur> point,
                             std::string &&name,
                             Args &&...args
        ) {
            auto future = std::async([=, name = std::move(name)]() {
                return this->DoRpcFunc<Res>(name, std::forward<Args>(args)...);
            });
            if (auto state = future.wait_until(point);state == std::future_status::timeout ||
                                                      state == std::future_status::deferred
                    ) {
                return {};
            }
            return future.get();
        }


    private:
        template<class Res, class ...Args>
        Res DoRpcFunc(const std::string &name, Args &&...args) {

            //=================== StringSerialize ===================//
            CLSN::RPCRequest request;
            request.funcName = name;
            request.async = static_cast<short>(CLSN::RpcType::Sync);
            {
                CLSN::StringSerialize encode(request.args);
                encode(std::forward_as_tuple(args...));
            }

            {
                std::string data;
                CLSN::StringSerialize encode(data);
                encode(request);
                //=================== send data ===================//
                if (-1 == TcpClient::Send(data)) {
                    throw std::logic_error(SeverIsBase);
                }
            }

            std::string_view view = TcpClient::Receive();
            CLSN::RPCResponse response;
            {
                CLSN::StringDeSerialize decode(view);
                decode(response);
            }
            CLSN::StringDeSerialize decode(response.res);
            if (response.succeed) {
                Res res;
                decode(res);
                return res;
            }

            throw std::logic_error(response.res);

            return {};

        }

    private:
        int a{};
    };

} // CLSN

#endif //DEFTRPC_RPCCLIENT_H
