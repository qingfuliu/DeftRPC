//
// Created by lqf on 23-6-24.
//
#include "common/Crc32.h"

namespace CLSN {

uint32_t GenerateCrc32(const char *data, std::size_t len) noexcept {
  uint32_t crc = 0;
  for (int i = 0; i < len; ++i) {
    auto c = data[i] & 0xff;
    crc = s_crc32Table[(crc >> 24) ^ c] ^ (crc << 8);
  }
  return crc;
}

bool CheckCec32(const char *data, std::size_t len) noexcept { return GenerateCrc32(data, len) == 0; }

}  // namespace CLSN