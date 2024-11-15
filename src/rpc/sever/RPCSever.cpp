//
// Created by lqf on 23-6-23.
//
#include "rpc/RPCSever.h"
#include "log/Log.h"
#include "rpc/RpcCodeC.h"

namespace clsn {

RPCSever::RPCSever(const std::string &ipPort, size_t sharedStackSize) noexcept
    : TcpSever(ipPort, sharedStackSize), m_router_(nullptr) {
  TcpSever::SetMagCallback([this](auto &&PH1, auto &&PH2, auto &&PH3) {
    return MessageCallBack(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2),
                           std::forward<decltype(PH3)>(PH3));
  });
  TcpSever::SetCodeC(new RpcCodeC);
}

void RPCSever::Start(int timeout) noexcept {
#ifdef DEBUG
  auto it = m_router_->GetIterator();
  CLSN_LOG_DEBUG
      << "============================================rpc functions============================================";
  while (it->IsValid()) {
    CLSN_LOG_DEBUG << HashTable::GetKeyByIterator(it.get());
    it->Next();
  }
  CLSN_LOG_DEBUG
      << "============================================rpc functions============================================";
#endif
  TcpSever::Start(timeout);
}

std::string RPCSever::MessageCallBack(clsn::TcpConnection *connection, std::string_view arg, clsn::TimeStamp t) {
  RPCResponse response;

  RPCRequest request;
  clsn::StringDeSerialize de_code(arg);
  try {
    de_code(request);
    if (request.m_async_ == static_cast<short>(kRpcType::Sync)) {
      response.m_res_ = m_router_->CallFuncSync(request.m_func_name_, request.m_args_);
      response.m_succeed_ = true;
    }
  } catch (std::exception &e) {
    response.m_succeed_ = false;
    response.m_res_ = e.what();
  }
  std::string res;
  clsn::StringSerialize en_code(res);
  en_code(response);
  return res;
}

}  // namespace clsn