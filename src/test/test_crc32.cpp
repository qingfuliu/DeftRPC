//
// Created by lqf on 23-6-24.
//
#include <cassert>
#include <string>
#include "common/Crc32.h"

#include <string.h>
#include <iostream>
#include <string>

using namespace std;
namespace detail {
/* 十进制转二进制 */
string dec_to_bin(int c) {
  if (c < 2) return string(1, c + '0');              /* 出口 */
  return dec_to_bin(c / 2) + string(1, c % 2 + '0'); /* 递归，短除法思路 */
}

/* 按char的8bits补齐前缀0 */
string bin_complete(string bin_code) { return string(8 - bin_code.length(), '0') + bin_code; }

/* 将一个char转化为二进制的string */
string char_to_bin(char c) {
  unsigned char u_c = c;
  return bin_complete(dec_to_bin(u_c));
}

void sting_stdout(std::string a) {
  for (auto c : a) {
    std::cout << char_to_bin(c);
  }
  std::cout << std::endl;
}

}  // namespace detail

void test_crcTable() noexcept {
  uint32_t p = 0x04c11db7;
  for (uint32_t byte = 0; byte < 256; ++byte) {
    uint32_t crc = byte << 24;
    for (int i = 8; i > 0; --i) {
      if (0 != (crc & 0x80000000)) {
        crc = (crc << 1) ^ p;
      } else {
        crc <<= 1;
      }
    }
    assert(CLSN::s_crc32Table[byte] == crc);
  }
}

void test_crcCheck_true() {
  std::string t;
  for (int i = 0; i < 1000; ++i) {
    t.append("lqf");
    auto k = CLSN::GenerateCrc32(t.data(), t.size());
    for (int i = 3; i >= 0; --i) {
      t.append(reinterpret_cast<char *>(&k) + i, 1);
    }
    assert(CLSN::CheckCec32(t.data(), t.size()));
  }
}

int main() {
  test_crcTable();
  test_crcCheck_true();
}