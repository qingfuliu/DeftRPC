//
// Created by lqf on 23-5-10.
//

#include "net/client/TcpClient.h"

namespace clsn {
    TcpClient::TcpClient(const std::string &ipPort) noexcept
            :
            m_remote_(ipPort) {}

    TcpClient::~TcpClient() noexcept {
        if (m_state_ == kState::Connected) {
            if (0 != Close()) {
                CLSN_LOG_ERROR << "close connect failed,error is " << strerror(errno);
            }
        }
    }
}  // namespace clsn