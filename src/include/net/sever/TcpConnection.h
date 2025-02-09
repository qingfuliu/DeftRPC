//
// Created by lqf on 23-5-9.
//

#ifndef DEFTRPC_TCPCONNECTION_H
#define DEFTRPC_TCPCONNECTION_H

#include <unistd.h>
#include <cerrno>
#include <memory>
#include <string>
#include "common/buffer/EVBuffer.h"
#include "common/buffer/RingBuffer.h"
#include "common/common.h"
#include "common/timer/Timer.h"
#include "coroutine/Coroutine.h"
#include "coroutine/Scheduler.h"
#include "log/Log.h"
#include "net/Addr.h"
#include "net/Socket.h"

namespace clsn {

class TcpSever;

class TcpConnection;

using MagCallback = std::function<std::string(TcpConnection *, std::string_view, clsn::TimeStamp)>;

class TcpConnection : public Noncopyable {
 public:
  explicit TcpConnection(int f, const Addr &addr, TcpSever *sever) noexcept;

  ~TcpConnection() override;

  void Close() {
    m_scheduler_->AddDefer([this] { m_socket_.Close(); });
  }

  static void NewTcpConnectionArrive(int fd, const Addr &remote, TcpSever *sever) noexcept;

 private:
  void ProcessMag();

 private:
  const Socket m_socket_;
  const Addr m_remote_addr_;
  std::unique_ptr<Buffer> m_input_buffer_;
  std::unique_ptr<Buffer> m_output_buffer_;
  Scheduler *const m_scheduler_ = Scheduler::GetThreadScheduler();
  TcpSever *const m_sever_;
};
}  // namespace clsn

#endif  // DEFTRPC_TCPCONNECTION_H
