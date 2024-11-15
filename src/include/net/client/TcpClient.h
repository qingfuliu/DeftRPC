//
// Created by lqf on 23-5-10.
//

#ifndef DEFTRPC_TCPCLIENT_H
#define DEFTRPC_TCPCLIENT_H

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <string_view>
#include "log/Log.h"
#include "net/Addr.h"
#include "net/Codec.h"
#include "net/EVBuffer.h"
#include "net/RingBuffer.h"
#include "net/Socket.h"

namespace clsn {

class TcpClient {
 public:
  enum class kState : std::int16_t { Construct, Connecting, Connected, ConnectFailed, ErrorOccurred, Closed };

 public:
  explicit TcpClient(const std::string &ipPort) noexcept;

  ~TcpClient() noexcept;

  bool Connect(int timeOut = 0) noexcept {
    if (m_state_ >= kState::Connected) {
      return m_state_ == kState::Connected;
    }
    if ((timeOut > 0 && 0 != SetReadTimeOut(timeOut)) || 0 != m_sock_.Connect(&m_remote_)) {
      m_state_ = kState::ConnectFailed;
      return false;
    }
    if (timeOut > 0) {
      return 0 != SetReadTimeOut(m_read_timeout_);
    }
    return true;
  }

  bool ConnectWithAddr(const Addr &addr, int timeOut = 0) noexcept {
    if (m_state_ >= kState::Connected) {
      return m_state_ == kState::Connected;
    }
    if ((timeOut > 0 && 0 != SetReadTimeOut(timeOut)) || 0 != m_sock_.Connect(&addr)) {
      m_state_ = kState::ConnectFailed;
      return false;
    }
    if (timeOut > 0) {
      return 0 != SetReadTimeOut(m_read_timeout_);
    }
    return true;
  }

  int Send(const std::string &str) noexcept { return SendAll(str.data(), str.size()); }

  int SendAll(const char *msg, size_t len) noexcept {
    if (len == 0) {
      return 0;
    }
    m_codec_->Encode(*m_output_buffer_, msg, len);
    int res = 0;
    int temp;
    do {
      temp = m_output_buffer_->WriteToFd(m_sock_.GetFd());
      res += temp;
    } while (temp >= 0 && !m_output_buffer_->IsEmpty());
    if (temp < 0) {
      return -1;
    }
    return res;
  }

  size_t Receive(char *data, size_t len) noexcept {
    if (len <= 0) {
      return 0;
    }
    int res = m_input_buffer_->ReadFromFd(m_sock_.GetFd());
    if (res > 0) {
      len = res > len ? len : res;
      m_input_buffer_->Read(data, static_cast<int>(len));
    }
    return res;
  }

  std::string_view Receive() {
    std::string_view view;
    do {
      int res = m_input_buffer_->ReadFromFd(m_sock_.GetFd());
      if (res < 0) {
        break;
      }
      view = m_codec_->Decode(*m_input_buffer_);
    } while (view.empty());
    return view;
  }

  [[nodiscard]] int MakeSelfNonblock(bool val) const noexcept { return m_sock_.SetNoBlock(val); }

  [[nodiscard]] int Close() noexcept {
    m_state_ = kState::Closed;
    return m_sock_.Close();
  }

  [[nodiscard]] kState GetClientState() const noexcept { return m_state_; }

  int SetReadTimeOut(int timeOut) noexcept {
    m_read_timeout_ = timeOut;
    int res = m_sock_.SetReadTimeout(timeOut);
    if (res != 0) {
      m_state_ = kState::ErrorOccurred;
      CLSN_LOG_ERROR << "set read timeout failed after connected,error is " << strerror(errno);
    }
    return res;
  }

  int SetWriteTimeOut(int timeOut) noexcept {
    m_write_timeout_ = timeOut;
    int res = m_sock_.SetWriteTimeout(timeOut);
    if (res != 0) {
      m_state_ = kState::ErrorOccurred;
      CLSN_LOG_ERROR << "set write timeout failed,error is " << strerror(errno);
    }
    return res;
  }

  Socket &GetSocket() noexcept { return m_sock_; }

  Addr &GetRemote() noexcept { return m_remote_; }

  void SetCodeC(CodeC *enCodeC) noexcept { m_codec_.reset(enCodeC); }

 private:
  Socket m_sock_{CreateBlockSocket()};
  Addr m_remote_;
  kState m_state_{kState::Construct};
  int m_read_timeout_{0};
  int m_write_timeout_{0};
  std::unique_ptr<CodeC> m_codec_{DefaultCodeCFactory::CreateCodeC()};
  std::unique_ptr<RingBuffer> m_input_buffer_{std::make_unique<RingBuffer>()};
  std::unique_ptr<EVBuffer> m_output_buffer_{std::make_unique<EVBuffer>()};
};

}  // namespace clsn

#endif  // DEFTRPC_TCPCLIENT_H
