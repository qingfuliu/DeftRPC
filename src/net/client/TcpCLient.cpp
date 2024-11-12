//
// Created by lqf on 23-5-10.
//

#include "net/client/TcpClient.h"

namespace clsn {
TcpClient::TcpClient(const std::string &ipPort) noexcept
    : sock(CreateBlockSocket()),
      remote(ipPort),
      state(State::Construct),
      readTimeout(0),
      writeTimeout(0),
      codeC(DefaultCodeCFactory::CreateCodeC()),
      inputBuffer(std::make_unique<RingBuffer>()),
      outputBuffer(std::make_unique<EVBuffer>()) {}

TcpClient::~TcpClient() noexcept {
  if (state == State::Connected) {
    if (0 != Close()) {
      CLSN_LOG_ERROR << "close connect failed,error is " << strerror(errno);
    }
  }
}
}  // namespace clsn