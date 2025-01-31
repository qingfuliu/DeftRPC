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
class TcpConnection;

using MagCallback = std::function<std::string(TcpConnection *, std::string_view, clsn::TimeStamp)>;

class TcpConnection : public Noncopyable {
 public:
  explicit TcpConnection(int f, const Addr &addr) noexcept;

  ~TcpConnection() override;

  void Write(const std::string &msg) noexcept { WriteInThread(msg); }

  void Write(const char *msg, size_t len) noexcept { WriteInThread(msg, len); }

  static void NewTcpConnectionArrive(int fd, const Addr &remote) noexcept;

 private:
  void WriteInThread(const std::string &msg) noexcept {
    assert(m_scheduler_->IsInLoopThread());
    WriteInThread(msg.c_str(), msg.size());
  }

  void WriteInThread(const char *msg, size_t len) noexcept {
    assert(m_scheduler_->IsInLoopThread());
    m_output_buffer_->Write(msg, len);
  }

  void ProcessMag();

 private:
  const Socket m_socket_;
  const Addr m_remote_addr_;
  std::unique_ptr<Buffer> m_input_buffer_;
  std::unique_ptr<Buffer> m_output_buffer_;
  Scheduler *const m_scheduler_ = Scheduler::GetThreadScheduler();
};
}  // namespace clsn

#endif  // DEFTRPC_TCPCONNECTION_H
