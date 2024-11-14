//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_TCPSEVER_H
#define DEFTRPC_TCPSEVER_H

#include <sys/socket.h>
#include <cerrno>
#include <cstring>
#include <unordered_map>
#include "coroutine/Scheduler.h"
#include "net/Addr.h"
#include "net/Socket.h"

namespace clsn {

class TcpConnection;

class CodeC;

class TcpSever : public Scheduler {
 public:
  using MagCallback = std::function<std::string(TcpConnection *, std::string_view, clsn::TimeStamp)>;

  explicit TcpSever(const std::string &ipPort, size_t sharedStackSize = 0, bool UserCall = true) noexcept;

  ~TcpSever() override;

  [[nodiscard]] CodeC *GetCodeC() noexcept { return m_codec_.get(); }

  void SetCodeC(CodeC *codeC) noexcept;

  void SetMagCallback(MagCallback callback) noexcept { m_msg_callback_ = std::move(callback); }

  MagCallback &GetMagCallback() noexcept { return m_msg_callback_; }

  void CloseConnection(int fd, bool activelyClose = false) noexcept;

  void Start(int timeout) noexcept override;

  void Stop() noexcept override;

 private:
  void AcceptTask() noexcept;

  void NewConnectionArrives(int fd, const Addr &remote) noexcept;

  void CloseAllConnection() noexcept;

  void CloseAcceptor() const noexcept;

 private:
  Socket m_accept_socket_;
  const Addr m_local_addr_;
  std::unique_ptr<CodeC> m_codec_;
  std::unique_ptr<Coroutine> m_accept_coroutine_;
  std::unordered_map<int, std::unique_ptr<Coroutine>> m_connections_;
  MagCallback m_msg_callback_;
};

}  // namespace clsn

#endif  // DEFTRPC_TCPSEVER_H
