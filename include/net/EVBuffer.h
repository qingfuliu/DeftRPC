//
// Created by lqf on 23-5-13.
//

#ifndef DEFTRPC_EVBUFFER_H
#define DEFTRPC_EVBUFFER_H

#include <memory>
#include <sys/uio.h>

namespace CLSN {
    /**
     * 不会持有任何空间,之持有指针
     * 在尾部写入不会分配空间
     */
    class EVBuffer {
    public:
        EVBuffer() noexcept:
                iovecs(std::make_unique<struct iovec[]>(3)),
                size(3),
                begin(0),
                end(0) {}

        ~EVBuffer() noexcept = default;


        void Write(const char *data, size_t len) noexcept {
            if (len == 0) {
                return;
            }
            if (end >= size) {
                if (begin != 0) {
                    std::copy(iovecs.get() + begin, iovecs.get() + end, iovecs.get() + begin - 1);
                    --end;
                } else {
                    ++size;
                    auto *temp = new struct iovec[size];
                    std::copy(iovecs.get(), iovecs.get() + end, temp);
                    iovecs.reset(temp);
                }
            }

            iovecs[end].iov_len = len;
            iovecs[end].iov_base = const_cast<char *>(data);
            ++end;
        }

        void Write(std::string &data) noexcept {
            Write(data.data(), data.size());
        }

        void WritePrepend(const char *data, size_t len) noexcept {
            if (len == 0) {
                return;
            }
            if (begin != 0) {
                --begin;
            } else if (end < size) {
                std::copy(iovecs.get() + begin, iovecs.get() + end, iovecs.get() + begin + 1);
                ++end;
            } else {
                ++size;
                auto *temp = new struct iovec[size + 1];
                std::copy(iovecs.get(), iovecs.get() + end, temp + 1);
                iovecs.reset(temp);
                ++end;
            }
            iovecs[begin].iov_base = const_cast<char *>(data);
            iovecs[begin].iov_len = len;
        }

        void WritePrepend(std::string &str) noexcept {
            WritePrepend(str.data(), str.size());
        }

        void WritePrepend(std::string_view view) noexcept {
            WritePrepend(const_cast<char *>(view.data()), view.size());
        }

        size_t ReadAll() noexcept {
            size_t res = 0;
            if (!IsEmpty()) {
                for (auto i = begin; i != end; i++) {
                    res += iovecs[i].iov_len;
                }
                begin = 0;
                end = 0;
            }
            return res;
        }

        int Read(int len) noexcept {
            if (IsEmpty()) {
                return 0;
            }
            int res = 0;
            while (begin != end) {
                if (len >= iovecs[begin].iov_len) {
                    res += static_cast<int>(iovecs[begin].iov_len);
                    len -= static_cast<int>(iovecs[begin].iov_len);
                    begin++;
                } else {
                    iovecs[begin].iov_base = static_cast<char *>(iovecs[begin].iov_base) + len;
                    iovecs[begin].iov_len -= len;
                    res += len;
                    break;
                }
            }
            if (begin == end) {
                begin = 0;
                end = 0;
            }
            size = end - begin;
            return res;
        }

        [[nodiscard]] const struct iovec *GetIovecs() const noexcept {
            if (IsEmpty()) {
                return nullptr;
            }
            return iovecs.get() + begin;
        }


        [[nodiscard]] int GetIovecsSize() const noexcept {
            return static_cast<int>(end - begin);
        }

        [[nodiscard]] bool IsEmpty() const noexcept {
            return begin == end;
        }

        void Clear() noexcept {
            begin = 0;
            end = 0;
        }

        int WriteToFd(int fd)noexcept;

    private:
        std::unique_ptr<struct iovec[]> iovecs;
        unsigned int size;
        unsigned int begin;
        unsigned int end;
    };


} // CLSN

#endif //DEFTRPC_EVBUFFER_H
