//
// Created by lqf on 23-5-9.
//

#ifndef DEFTRPC_TCPCONNECTION_H
#define DEFTRPC_TCPCONNECTION_H

#include <unistd.h>
#include <cerrno>
#include <cstring>
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
  explicit TcpConnection(int fd, const Addr &addr, TcpSever *sever,
                         MultiThreadScheduler::SchedulerThread *scheduler) noexcept;

  ~TcpConnection() override;

  void Close();
  /**
   * Called after tcpconnection is constructed
   */
  void NewTcpConnectionEstablish() noexcept;

 private:
  void ProcessMag();

 private:
  Socket m_socket_;
  const Addr m_remote_addr_;
  std::unique_ptr<Buffer> m_input_buffer_;
  std::unique_ptr<Buffer> m_output_buffer_;
  std::unique_ptr<Coroutine> m_coroutine_;
  MultiThreadScheduler::SchedulerThread *const m_scheduler_thread_;
  Scheduler *const m_scheduler_;
  TcpSever *const m_sever_;
};
}  // namespace clsn

#endif  // DEFTRPC_TCPCONNECTION_H
