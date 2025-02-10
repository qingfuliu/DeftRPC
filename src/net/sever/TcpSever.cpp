//
// Created by lqf on 23-5-10.
//

#include "net/sever/TcpSever.h"
#include "common/codeC/Codec.h"
#include "hook/Hook.h"
#include "net/sever/TcpConnection.h"

namespace clsn {

TcpSever::TcpSever(const std::string &ipPort, std::uint32_t threadNumber, size_t sharedStackSize) noexcept
    : MultiThreadScheduler(threadNumber, sharedStackSize),
      m_accept_socket_(CreateNoBlockSocket()),
      m_local_addr_(ipPort),
      m_codec_(DefaultCodeCFactory::CreateCodeC()),
      m_accept_coroutine_(CreateCoroutine([this]() -> void {
        this->AcceptTask();
        Scheduler::Terminal();
      })) {
  m_accept_socket_.SetReusePort(true);
  m_accept_socket_.SetReuseAddr(true);
  if (m_codec_ == nullptr) {
    m_codec_.reset(DefaultCodeCFactory::CreateCodeC());
  }
}

TcpSever::~TcpSever() {
  if (!m_stop_.load(std::memory_order_release)) {
    Scheduler::CancelRegister(m_accept_socket_.GetFd());
    clsn::MultiThreadScheduler::Stop();
  }
}

void TcpSever::SetCodeC(CodeC *codeC) noexcept { m_codec_.reset(codeC); }

void TcpSever::CleanConnection(int fd) noexcept {
  CLSN_LOG_ERROR << "clean a connection:" << fd;
  auto it = m_connections_.find(fd);
  if (m_connections_.end() == it) {
    CLSN_LOG_ERROR << "clean a not exist connection!";
    return;
  }
  if (0 != close(fd)) {
    CLSN_LOG_ERROR << "close socket " << fd
                   << "error when "
                      "CleanConnection,error is "
                   << errno;
  }
  m_connections_.erase(it);
}

void TcpSever::Start(int timeout) noexcept {
  Scheduler::AddDefer([this]() { Scheduler::SwapIn(m_accept_coroutine_.get()); });
  clsn::MultiThreadScheduler::Start(timeout);
  CloseAcceptor();
  CloseAllConnection();
}

void TcpSever::Stop() noexcept {
  if (IsInLoopThread()) {
    Scheduler::CancelRegister(m_accept_socket_.GetFd());
    clsn::MultiThreadScheduler::Stop();
  } else {
    AddDefer([this]() { this->Stop(); });
  }
}

void TcpSever::AcceptTask() noexcept {
  if (m_accept_socket_.Bind(&m_local_addr_) != 0) {
    CLSN_LOG_ERROR << "m_accept_socket_ bind failed,error is " << strerror(errno) << ",sever will m_stop_ in 3s";
    DoAfter(std::chrono::seconds{3}, [this] { Stop(); });
    return;
  }
  if (m_accept_socket_.Listen() != 0) {
    CLSN_LOG_ERROR << "m_accept_socket_ listen failed,error is " << strerror(errno) << ",sever will m_stop_ in 3s";
    DoAfter(std::chrono::seconds{3}, [this] { Stop(); });
    return;
  }
  CLSN_LOG_INFO << "Sever is ready to accept";
  do {
    Addr remote;
    int fd = m_accept_socket_.Accept(&remote);
    if (fd > 0) {
      auto scheduler = GetNextScheduler();
      auto it = m_connections_.emplace(fd, CreateCoroutine(
                                               [fd, remote, sever = this, scheduler]() {
                                                 TcpConnection::NewTcpConnectionArrive(fd, remote, sever, scheduler);
                                                 Scheduler::Terminal();
                                               },
                                               scheduler->GetSharedStack()));
      scheduler->AddDefer([coroutine = it.first->second.get()]() { Scheduler::SwapIn(coroutine); });
    } else if (fd == -1 && errno == EBADF) {
      CLSN_LOG_ERROR << "please make sure this message only appears when the server is shut down!";
      break;
    } else if (fd == -1 && errno == EAGAIN) {
    } else {
      CLSN_LOG_ERROR << "accept error,error is " << strerror(errno);
    }
  } while (!m_stop_.load(std::memory_order_acquire));
  CLSN_LOG_DEBUG << "acceptor exit";
}

void TcpSever::NewConnectionArrives(int fd, const Addr &remote) noexcept {}

void TcpSever::CloseAllConnection() noexcept {
  for (auto &[fd, c] : m_connections_) {
    close(fd);
    Scheduler::SwapIn(c.get());
  }
  m_connections_.clear();
}

void TcpSever::CloseAcceptor() noexcept {
  close(m_accept_socket_.GetFd());
  Scheduler::AddDefer([accept_ptr = m_accept_coroutine_.get()] { Scheduler::SwapIn(accept_ptr); });
}

}  // namespace clsn