//
// Created by lqf on 23-1-5.
//

#ifndef MCLOUDDISK_ADDR_H
#define MCLOUDDISK_ADDR_H

#include<string>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<variant>

namespace CLSN {
    class Addr {
        static int FromString(const std::string &ip, uint16_t port, struct sockaddr_in *);

        static int FromString(const std::string &ip, uint16_t port, struct sockaddr_in6 *);

    public:
        Addr() noexcept = default;

        explicit Addr(const std::string &ipPort, bool ipv4 = true) noexcept;

        Addr(const std::string &ip, uint16_t port, bool ipv4 = true) noexcept;

        explicit Addr(const sockaddr_in *addr) noexcept {
            this->address = *addr;
        }

        explicit Addr(const sockaddr_in6 *addr6) {
            this->address = *addr6;
        }

        Addr(const Addr &addr) noexcept {
            this->address = addr.address;
        }

        [[nodiscard]] sa_family_t getFamily() const noexcept {
            switch (address.index()) {
                case 0:
                    return std::get<0>(address).sin6_family;
                default:
                    return std::get<1>(address).sin_family;
            }
        }

        [[nodiscard]] size_t getSockAddrSize() const noexcept {
            switch (address.index()) {
                case 0:
                    return sizeof(sockaddr_in6);
                default:
                    return sizeof(sockaddr_in);
            }
        }

        sockaddr *getSockAddr() noexcept {
            return reinterpret_cast<sockaddr *>(&address);
        }

        [[nodiscard]] const sockaddr *getSockAddr() const noexcept {
            return reinterpret_cast<const sockaddr *>(&address);
        }

        [[nodiscard]] std::string toString() const noexcept;

    private:

        std::variant<sockaddr_in6, sockaddr_in> address{};

    };
}


#endif //MCLOUDDISK_ADDR_H
