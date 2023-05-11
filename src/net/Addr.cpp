//
// Created by lqf on 23-1-5.
//

#include "log/Log.h"
#include "net/Addr.h"
#include<string_view>
#include<algorithm>
#include<iostream>
#include<ostream>
#include<sstream>

namespace CLSN {
    int Addr::FromString(const std::string &ip, uint16_t port, struct sockaddr_in *addr) {
        std::fill(reinterpret_cast<char *>(addr), reinterpret_cast<char *>(addr) + sizeof(sockaddr_in), 0);
        addr->sin_family = AF_INET;
        if (inet_pton(AF_INET, ip.c_str(), &addr->sin_addr) != 1) {
            CLSN_LOG_ERROR << "Addr FromString failed!";
            //log
        }
        addr->sin_port = htons(port);
    }

    int Addr::FromString(const std::string &ip, uint16_t port, struct sockaddr_in6 *addr6) {
        std::fill(reinterpret_cast<char *>(addr6), reinterpret_cast<char *>(addr6) + sizeof(sockaddr_in6), 0);

        addr6->sin6_family = AF_INET6;
        if (inet_pton(AF_INET6, ip.c_str(), &addr6->sin6_addr) != 1) {
            CLSN_LOG_ERROR << "Addr FromString failed!";
        }
        addr6->sin6_port = htons(port);
    }

    Addr::Addr(const std::string &ipPort, bool ipv4) noexcept {
        do {
            uint16_t pos = static_cast<uint16_t>(ipPort.find(':', 0));
            if (pos == ipPort.npos) {
                CLSN_LOG_ERROR << "ipPort is invalid!";
                break;
            }
            auto temp = std::string_view(ipPort.c_str() + pos + 1);
            uint16_t port = static_cast<uint16_t>(atoi(temp.data()));
            if (ipv4) {
                address = sockaddr_in{};
                FromString(std::string(ipPort.begin(), ipPort.begin() + pos), port, &std::get<1>(address));
            } else {
                address = sockaddr_in6{};
                FromString(std::string(ipPort.begin(), ipPort.begin() + pos), port, &std::get<0>(address));
            }
        } while (0);
    }

    Addr::Addr(const std::string &ip, uint16_t port, bool ipv4) noexcept {

        if (ipv4) {
            FromString(ip, port, &std::get<1>(address));
        } else {
            FromString(ip, port, &std::get<0>(address));
        }

    }

    std::string Addr::toString() const noexcept {
        char *buf;
        uint16_t port = 0;
        std::ostringstream oss;
        do {
            if (1 == address.index()) {
                auto &addr = std::get<1>(address);
                buf = static_cast<char *>(malloc(INET_ADDRSTRLEN));
                if (inet_ntop(AF_INET, &addr.sin_addr, buf, INET_ADDRSTRLEN) == NULL) {
                    //log
                    std::cout << "inet_ntop failed!! line: " << __LINE__ << " file: " << __FILE__ << std::endl;
                    break;
                }
                oss << buf << ":" << ntohs(addr.sin_port);
            } else {
                auto &addr6 = std::get<0>(address);
                buf = static_cast<char *>(malloc(INET6_ADDRSTRLEN));
                if (inet_ntop(AF_INET6, &addr6.sin6_addr, buf, INET6_ADDRSTRLEN) == NULL) {
                    //log
                    std::cout << "inet_ntop failed!! line: " << __LINE__ << " file: " << __FILE__ << std::endl;
                    break;
                }
                oss << buf << ":" << ntohs(addr6.sin6_port);
            }
        } while (0);
        free(buf);
        return oss.str();
    }
}
