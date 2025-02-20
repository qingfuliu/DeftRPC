//
// Created by lqf on 23-5-9.
//

#include "net/sever/TcpConnection.h"
#include "common/codeC/Codec.h"
#include "common/timer/Timer.h"
#include "net/sever/TcpSever.h"

namespace clsn {

TcpConnection::TcpConnection(int fd, const Addr &addr, TcpSever *sever,
                             MultiThreadScheduler::SchedulerThread *scheduler) noexcept
    : m_socket_(fd),
      m_remote_addr_(addr),
      m_input_buffer_(std::make_unique<RingBuffer>()),
      m_output_buffer_(std::make_unique<RingBuffer>()),
      m_coroutine_(CreateCoroutine(
          [this] {
            this->ProcessMag();
            Scheduler::Terminal();
          },
          scheduler->GetScheduler()->GetSharedStack(), false)),
      m_scheduler_thread_(scheduler),
      m_scheduler_(scheduler->GetScheduler()),
      m_sever_(sever) {}

TcpConnection::~TcpConnection() = default;

void TcpConnection::Close() {
  m_scheduler_->AddDefer([this] {
    if (0 != m_socket_.Close()) {
      CLSN_LOG_ERROR << "socket close error!" << " Context[" << "fd:" << m_socket_.GetFd()
                     << ", error:" << strerror(errno) << "]";
      throw std::runtime_error("socket close error!");
    }
    Scheduler::SwapIn(m_coroutine_.get());
    m_sever_->CleanConnection(m_scheduler_thread_->GetIndex(), m_socket_.GetFd());
  });
}

void TcpConnection::NewTcpConnectionEstablish() noexcept {
  m_scheduler_->AddDefer([coroutine = m_coroutine_.get()]() { Scheduler::SwapIn(coroutine); });
}

void TcpConnection::ProcessMag() {
  std::string exception;
  do {
    int res;
    do {
      res = m_input_buffer_->FetchDataFromFd(m_socket_.GetFd());
      if (res <= 0) {
        break;
      }
      try {
        std::string view = m_sever_->GetCodeC()->Decode(m_input_buffer_.get());
        if (!view.empty()) {
          std::string response_msg = m_sever_->GetMagCallback()(this, view, TimeStamp::Now());
          m_sever_->GetCodeC()->Encode(m_output_buffer_.get(), response_msg);
          m_output_buffer_->FlushDataToFd(m_socket_.GetFd());
        }
      } catch (std::exception &e) {
        exception = e.what();
        m_sever_->GetCodeC()->Encode(m_output_buffer_.get(), exception);
        m_output_buffer_->FlushDataToFd(m_socket_.GetFd());
      }
    } while (true);

    // remote close
    if (res == 0) {
      if (!m_input_buffer_->Empty()) {
        m_output_buffer_->FlushDataToFd(m_socket_.GetFd());
      }

      m_scheduler_->AddDefer([sever = this->m_sever_, scheduler_thread = this->m_scheduler_thread_,
                              fd = m_socket_.GetFd()] {
        sever->CleanConnection(scheduler_thread->GetIndex(), fd);
        if (0 != close(fd)) {
          CLSN_LOG_ERROR << "close socket error!" << " Context[" << "fd:" << fd << ", error:" << strerror(errno) << "]";
          throw std::runtime_error("close socket error!");
        }
      });
      break;
    }
    // local close
    if (res == -1 && errno == EBADF) {
      CLSN_LOG_WARNING << "please make sure this message only appears when the server is shut down or local close!";
      break;
    } else {
      CLSN_LOG_ERROR << "read from m_socket_ error,error is " << strerror(errno) << ", code:" << errno;
      break;
    }
  } while (!m_scheduler_->IsStop());
}
}  // namespace clsn
