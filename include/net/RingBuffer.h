//
// Created by lqf on 23-1-11.
//

#ifndef MCLOUDDISK_MCLOUDBUFFER_H
#define MCLOUDDISK_MCLOUDBUFFER_H

#include<vector>
#include<string>
#include<memory>
#include <arpa/inet.h>

namespace CLSN {
    class RingBuffer {
    public:
        RingBuffer() noexcept;

        ~RingBuffer() = default;

        [[nodiscard]] uint32_t GetPackageLength() const noexcept;

        char *ReadAll(int *len = nullptr) noexcept;

        char *Read(int size, int *len = nullptr) noexcept;

        int Read(char *buf, int len) noexcept;

        [[nodiscard]] int GetReadableCapacity() const noexcept {
            return size;
        }

        int ReadFromFd(int fd) noexcept;

        int WriteToFd(int fd) noexcept;

        int Write(const char *buf, int len) noexcept;

        int Write(const std::string &buf) noexcept { return this->Write(buf.c_str(), static_cast<int>(buf.size())); }

        void Append(const std::string &buf) noexcept { Write(buf); }

        void Append(const char *buf, int len) noexcept { Write(buf, len); }

        [[nodiscard]] bool IsEmpty() const noexcept { return begin == end && size == 0; }

    private:

        [[nodiscard]] bool isFull() const noexcept { return begin == end && size == buffer.capacity(); }

        [[nodiscard]] int getTailSpaceLen() const noexcept;

        [[nodiscard]] int getTailContentLen() const noexcept;

        [[nodiscard]] int getWritableCapacity() const noexcept;

        void updateAfterRead(int len) noexcept;

        void updateAfterWrite(int len) noexcept;

        void enableWritableSpace(int targetSize) noexcept;

    private:
        int begin;
        int end;
        int size;
        std::vector<char> buffer;
        int tempCapacity;
        std::unique_ptr<char[]> temp;
    };

}

#endif //MCLOUDDISK_MCLOUDBUFFER_H
