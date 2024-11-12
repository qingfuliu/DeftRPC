//
// Created by lqf on 23-5-10.
//

#include "net/sever/TcpSever.h"
#include "hook/Hook.h"
#include "net/Codec.h"
#include "net/sever/TcpConnection.h"

namespace clsn {

TcpSever::TcpSever(const std::string &ipPort, size_t sharedStackSize, bool UserCall) noexcept
    : Scheduler(sharedStackSize, UserCall),
      acceptSock(CreateNoBlockSocket()),
      local(ipPort),
      mCodeC(DefaultCodeCFactory::CreateCodeC()),
      acceptCoroutine(CreateCoroutine([this]() -> void { this->acceptTask(); })) {
  acceptSock.SetReusePort(true);
  acceptSock.SetReuseAddr(true);

  if (mCodeC == nullptr) {
    mCodeC.reset(DefaultCodeCFactory::CreateCodeC());
  }
}

TcpSever::~TcpSever() {
  if (!stop.load(std::memory_order_release)) {
    AddTask([this]() { closeAcceptor(); });

    AddTask([this]() { closeAllConnection(); });
    Scheduler::Stop();
  }
}

void TcpSever::SetCodeC(CodeC *codeC) noexcept { mCodeC.reset(codeC); }

void TcpSever::CloseConnection(int fd, bool activelyClose) noexcept {
  if (IsInLoopThread()) {
    auto it = connections.find(fd);
    if (connections.end() == it) {
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
      it->second->swapIn();
    }
    connections.erase(it);
  } else {
    AddTask([this, fd] { CloseConnection(fd); });
  }
}

void TcpSever::Start(int timeout) noexcept {
  int a = 0;
  Scheduler::AddDefer([this]() { acceptCoroutine->swapIn(); });
  Scheduler::Start(timeout);
}

void TcpSever::Stop() noexcept {
  if (IsInLoopThread()) {
    Scheduler::Stop();
    closeAcceptor();
    closeAllConnection();
  } else {
    AddTask([this]() { Stop(); });
  }
}

void TcpSever::acceptTask() noexcept {
  if (acceptSock.Bind(&local) != 0) {
    CLSN_LOG_ERROR << "acceptSock bind failed,error is " << strerror(errno) << ",sever will stop in 3s";
    DoAfter(std::chrono::seconds{3}, [this] { Stop(); });
    return;
  }
  if (acceptSock.Listen() != 0) {
    CLSN_LOG_ERROR << "acceptSock listen failed,error is " << strerror(errno) << ",sever will stop in 3s";
    DoAfter(std::chrono::seconds{3}, [this] { Stop(); });
    return;
  }
  CLSN_LOG_DEBUG << "Sever is ready to accept";
  do {
    Addr remote;
    int fd = acceptSock.Accept(&remote);
    if (fd > 0) {
      this->newConnectionArrives(fd, remote);
    } else if (fd == -1 && errno == EBADF) {
      CLSN_LOG_ERROR << "please make sure this message only appears when the server is shut down!";
      assert(stop.load(std::memory_order_acquire));
    } else if (fd == -1 && errno == EAGAIN) {
    } else {
      CLSN_LOG_ERROR << "accept error,error is " << strerror(errno);
    }
  } while (!stop.load(std::memory_order_acquire));
  CLSN_LOG_DEBUG << "acceptor exit";
}

void TcpSever::newConnectionArrives(int fd, const Addr &remote) noexcept {
  auto it = connections.emplace(
      fd,
      CreateCoroutine([fd, remote]() { TcpConnection::NewTcpConnectionArrive(fd, remote); }, GetThreadSharedStack()));
  auto coroutine = it.first->second.get();
  AddDefer([coroutine]() { coroutine->swapIn(); });
}

void TcpSever::closeAllConnection() noexcept {
  std::vector<int> temp(connections.size());
  std::for_each(connections.begin(), connections.end(), [&temp](auto &x) { temp.push_back(x.first); });
  for (auto &fd : temp) {
    CloseConnection(fd, true);
  }
}

void TcpSever::closeAcceptor() const noexcept {
  close(acceptSock.getFd());
  acceptCoroutine->swapIn();
}

}  // namespace clsn