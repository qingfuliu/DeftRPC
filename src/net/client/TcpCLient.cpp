//
// Created by lqf on 23-5-10.
//

#include "net/client/TcpClient.h"

namespace clsn {
TcpClient::TcpClient(const std::string &ipPort) noexcept
    : m_sock_(CreateBlockSocket()),
      m_remote_(ipPort),
      m_state_(kState::Construct),
      m_read_timeout_(0),
      m_write_timeout_(0),
      m_codec_(DefaultCodeCFactory::CreateCodeC()),
      m_input_buffer_(std::make_unique<RingBuffer>()),
      m_output_buffer_(std::make_unique<EVBuffer>()) {}

TcpClient::~TcpClient() noexcept {
  if (m_state_ == kState::Connected) {
    if (0 != Close()) {
      CLSN_LOG_ERROR << "close connect failed,error is " << strerror(errno);
    }
  }
}
}  // namespace clsn