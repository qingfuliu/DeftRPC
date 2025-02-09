//
// Created by lqf on 23-5-9.
//

#include "net/sever/TcpConnection.h"
#include "common/codeC/Codec.h"
#include "common/timer/Timer.h"
#include "net/sever/TcpSever.h"

namespace clsn {

TcpConnection::TcpConnection(int f, const Addr &addr, TcpSever *sever) noexcept
    : m_socket_(f),
      m_remote_addr_(addr),
      m_input_buffer_(std::make_unique<RingBuffer>()),
      m_output_buffer_(std::make_unique<RingBuffer>()),
      m_sever_(sever) {}

TcpConnection::~TcpConnection() = default;

void TcpConnection::NewTcpConnectionArrive(int fd, const Addr &remote, TcpSever *sever) noexcept {
  TcpConnection connection(fd, remote, sever);
  connection.ProcessMag();
  Scheduler::Terminal();
}

void TcpConnection::ProcessMag() {
  auto sever = m_sever_;
  std::string exception;
  assert(nullptr != sever);
  do {
    int res;
    do {
      res = m_input_buffer_->FetchDataFromFd(m_socket_.GetFd());
      if (res <= 0) {
        break;
      }
      try {
        std::string view = sever->GetCodeC()->Decode(m_input_buffer_.get());
        if (!view.empty()) {
          std::string response_msg = sever->GetMagCallback()(this, view, TimeStamp::Now());
          sever->GetCodeC()->Encode(m_output_buffer_.get(), response_msg);
          m_output_buffer_->FlushDataToFd(m_socket_.GetFd());
        }
      } catch (std::exception &e) {
        exception = e.what();
        sever->GetCodeC()->Encode(m_output_buffer_.get(), exception);
        m_output_buffer_->FlushDataToFd(m_socket_.GetFd());
      }
    } while (true);

    // remote close
    if (res == 0) {
      if (!m_input_buffer_->Empty()) {
        m_output_buffer_->FlushDataToFd(m_socket_.GetFd());
      }
      //      CLSN_LOG_DEBUG << "remote close:" << m_socket_.GetFd();
      sever->AddDefer([this, sever, fd = m_socket_.GetFd()]() { sever->CleanConnection(fd); });
      break;
    }
    // local close
    if (res == -1 && errno == EBADF) {
      CLSN_LOG_ERROR << "please make sure this message only appears when the server is shut down or local close!";
      break;
    } else {
      CLSN_LOG_ERROR << "read from m_socket_ error,error is " << strerror(errno);
      sever->AddDefer([this, sever, fd = m_socket_.GetFd()]() { sever->CleanConnection(fd); });
      break;
    }
  } while (!sever->IsStop());
}
}  // namespace clsn
