//
// Created by lqf on 23-5-9.
//

#ifndef DEFTRPC_TCPCONNECTION_H
#define DEFTRPC_TCPCONNECTION_H

#include <unistd.h>
#include <cerrno>
#include <memory>
#include "common/common.h"
#include "coroutine/Coroutine.h"
#include "coroutine/Scheduler.h"
#include "coroutine/Timer.h"
#include "log/Log.h"
#include "net/Addr.h"
#include "net/EVBuffer.h"
#include "net/RingBuffer.h"
#include "net/Socket.h"

namespace clsn {
class TcpConnection;

using MagCallback = std::function<std::string(TcpConnection *, std::string_view, clsn::TimeStamp)>;

class TcpConnection : public noncopyable {
 public:
  explicit TcpConnection(int f, const Addr &addr) noexcept;

  ~TcpConnection() override;

  void Write(const std::string &msg) noexcept { writeInThread(msg); }

  void Write(const char *msg, size_t len) noexcept { writeInThread(msg, len); }

  static void NewTcpConnectionArrive(int fd, const Addr &remote) noexcept;

 private:
  void writeInThread(const std::string &msg) noexcept {
    assert(mScheduler->IsInLoopThread());
    writeInThread(msg.c_str(), msg.size());
  }

  void writeInThread(const char *msg, size_t len) noexcept {
    assert(mScheduler->IsInLoopThread());
    //            outputBuffer->Write(msg, len);
  }

  void processMag();

 private:
  const Socket sock;
  const Addr remote;
  std::unique_ptr<RingBuffer> inputBuffer;
  std::unique_ptr<EVBuffer> outputBuffer;
  Scheduler *const mScheduler = Scheduler::GetThreadScheduler();
};
}  // namespace clsn

#endif  // DEFTRPC_TCPCONNECTION_H
