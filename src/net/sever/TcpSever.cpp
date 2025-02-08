//
// Created by lqf on 23-5-10.
//

#include "net/sever/TcpSever.h"
#include "common/codeC/Codec.h"
#include "hook/Hook.h"
#include "net/sever/TcpConnection.h"

namespace clsn {

TcpSever::TcpSever(const std::string &ipPort, size_t sharedStackSize) noexcept
    : MultiThreadScheduler(8, sharedStackSize),
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
    AddTask([this]() { CloseAcceptor(); });

    AddTask([this]() { CloseAllConnection(); });
    clsn::MultiThreadScheduler::Stop();
  }
}

void TcpSever::SetCodeC(CodeC *codeC) noexcept { m_codec_.reset(codeC); }

void TcpSever::CloseConnection(int fd, bool activelyClose) noexcept {
  if (IsInLoopThread()) {
    auto it = m_connections_.find(fd);
    if (m_connections_.end() == it) {
      CLSN_LOG_ERROR << "close a not exist connection!";
      return;
    }
    if (activelyClose) {
      if (0 != close(fd)) {
        CLSN_LOG_ERROR << "close socket " << fd
                       << "error when "
                          "~connection,error is "
                       << errno;
      }
      Scheduler::SwapIn(it->second.get());
    }
    m_connections_.erase(it);
  } else {
    AddTask([this, fd] { CloseConnection(fd); });
  }
}

void TcpSever::Start(int timeout) noexcept {
  Scheduler::AddDefer([this]() { Scheduler::SwapIn(m_accept_coroutine_.get()); });
  clsn::MultiThreadScheduler::Start(timeout);
}

void TcpSever::Stop() noexcept {
  if (IsInLoopThread()) {
    clsn::MultiThreadScheduler::Stop();
    CloseAcceptor();
    CloseAllConnection();
  } else {
    AddTask([this]() { Stop(); });
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
  CLSN_LOG_DEBUG << "Sever is ready to accept";
  do {
    Addr remote;
    int fd = m_accept_socket_.Accept(&remote);
    if (fd > 0) {
      CLSN_LOG_DEBUG << "receive connection!" << fd;
      AddDefer([this, fd, remote] { this->NewConnectionArrives(fd, remote); });
    } else if (fd == -1 && errno == EBADF) {
      CLSN_LOG_ERROR << "please make sure this message only appears when the server is shut down!";
      assert(m_stop_.load(std::memory_order_acquire));
    } else if (fd == -1 && errno == EAGAIN) {
    } else {
      CLSN_LOG_ERROR << "accept error,error is " << strerror(errno);
    }
  } while (!m_stop_.load(std::memory_order_acquire));
  CLSN_LOG_DEBUG << "acceptor exit";
}

void TcpSever::NewConnectionArrives(int fd, const Addr &remote) noexcept {
  auto it = m_connections_.emplace(fd, CreateCoroutine(
                                           [fd, remote]() {
                                             TcpConnection::NewTcpConnectionArrive(fd, remote);
                                             Scheduler::Terminal();
                                           },
                                           GetThreadSharedStack()));
  auto coroutine = it.first->second.get();
  AddDefer([coroutine]() { Scheduler::SwapIn(coroutine); });
}

void TcpSever::CloseAllConnection() noexcept {
  std::vector<int> temp(m_connections_.size());
  std::for_each(m_connections_.begin(), m_connections_.end(), [&temp](auto &x) { temp.push_back(x.first); });
  for (auto &fd : temp) {
  }
}

void TcpSever::CloseAcceptor() const noexcept { close(m_accept_socket_.GetFd()); }

}  // namespace clsn