//
// Created by lqf on 23-5-9.
//

#include "net/sever/TcpConnection.h"
#include "coroutine/Timer.h"
#include "net/Codec.h"
#include "net/sever/TcpSever.h"

namespace clsn {

TcpConnection::TcpConnection(int f, const Addr &addr) noexcept
    : Noncopyable(),
      m_socket_(f),
      m_remote_addr_(addr),
      m_input_buffer_(std::make_unique<RingBuffer>()),
      m_output_buffer_(std::make_unique<EVBuffer>()) {}

TcpConnection::~TcpConnection() = default;

void TcpConnection::NewTcpConnectionArrive(int fd, const Addr &remote) noexcept {
  CLSN_LOG_DEBUG << "new connection arrive,remote address is " << remote.toString();
  TcpConnection connection(fd, remote);
  connection.ProcessMag();
  CLSN_LOG_DEBUG << "connection close,coroutine deconstruct";
}

void TcpConnection::ProcessMag() {
  auto mSever = dynamic_cast<TcpSever *>(m_scheduler_);
  std::string exception;
  assert(nullptr != mSever);
  do {
    int res;
    do {
      res = m_input_buffer_->ReadFromFd(m_socket_.getFd());
      if (res <= 0) {
        break;
      }
      try {
        std::string_view view = mSever->GetCodeC()->Decode(*m_input_buffer_);
        std::string responseMsg = mSever->GetMagCallback()(this, view, TimeStamp::Now());
        mSever->GetCodeC()->Encode(*m_output_buffer_, responseMsg);
        if (res < 0) {
          break;
        }
      } catch (std::exception &e) {
        exception = e.what();
        m_output_buffer_->Write(exception);
      }

      do {
        res = m_output_buffer_->WriteToFd(m_socket_.getFd());
      } while (!m_output_buffer_->IsEmpty() && res >= 0);

    } while (true);

    if (res == 0) {
      if (-1 == m_socket_.Close()) {
        CLSN_LOG_ERROR << "connection close error,error is " << strerror(errno);
      }
      int fd = m_socket_.getFd();
      mSever->AddDefer([mSever, fd]() { mSever->CloseConnection(fd); });
      break;
    } else if (res == -1 && errno == EBADF) {
      CLSN_LOG_ERROR << "please make sure this message only appears when the server is shut down!";
      assert(mSever->IsStop());
    } else {
      CLSN_LOG_ERROR << "read from m_socket_ error,error is " << strerror(errno);
    }
  } while (!mSever->IsStop());
}
}  // namespace clsn
