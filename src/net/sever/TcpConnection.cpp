//
// Created by lqf on 23-5-9.
//

#include "net/sever/TcpConnection.h"
#include "common/codeC/Codec.h"
#include "coroutine/Timer.h"
#include "net/sever/TcpSever.h"

namespace clsn {

TcpConnection::TcpConnection(int f, const Addr &addr) noexcept
    : m_socket_(f),
      m_remote_addr_(addr),
      m_input_buffer_(std::make_unique<RingBuffer>()),
      m_output_buffer_(std::make_unique<RingBuffer>()) {}

TcpConnection::~TcpConnection() = default;

void TcpConnection::NewTcpConnectionArrive(int fd, const Addr &remote) noexcept {
  CLSN_LOG_DEBUG << "new connection arrive,remote address is " << remote.ToString();
  TcpConnection connection(fd, remote);
  connection.ProcessMag();
  CLSN_LOG_DEBUG << "connection close,coroutine deconstruct";
}

void TcpConnection::ProcessMag() {
  auto sever = dynamic_cast<TcpSever *>(m_scheduler_);
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
        std::string response_msg = sever->GetMagCallback()(this, view, TimeStamp::Now());
        sever->GetCodeC()->Encode(m_output_buffer_.get(), response_msg);
      } catch (std::exception &e) {
        exception = e.what();
        m_output_buffer_->Write(exception);
      }

      do {
        res = m_output_buffer_->FlushDataToFd(m_socket_.GetFd());
      } while (!m_output_buffer_->Empty() && res >= 0);

    } while (true);

    if (res == 0) {
      if (-1 == m_socket_.Close()) {
        CLSN_LOG_ERROR << "connection close error,error is " << strerror(errno);
      }
      int fd = m_socket_.GetFd();
      sever->AddDefer([sever, fd]() { sever->CloseConnection(fd); });
      break;
    }
    if (res == -1 && errno == EBADF) {
      CLSN_LOG_ERROR << "please make sure this message only appears when the server is shut down!";
      assert(sever->IsStop());
    } else {
      CLSN_LOG_ERROR << "read from m_socket_ error,error is " << strerror(errno);
    }
  } while (!sever->IsStop());
}
}  // namespace clsn
