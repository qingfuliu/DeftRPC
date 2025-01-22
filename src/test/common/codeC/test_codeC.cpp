//
// Created by root on 1/20/25.
//

//
// Created by lqf on 23-4-30.
//
#include <gtest/gtest.h>
#include <memory>
#include <random>
#include "common/buffer/RingBuffer.h"
#include "common/codeC/Codec.h"

TEST(test_codeC, test_codeC1) {
  std::unique_ptr<clsn::Buffer> buffer = std::make_unique<clsn::RingBuffer>();
  std::unique_ptr<clsn::CodeC> codeC = std::make_unique<clsn::DefaultCodeC>();

  std::random_device randomDevice;
  unsigned int seed = randomDevice();  // 生成一个随机的种子值
  std::uniform_int_distribution<std::uint32_t> uniform(2, 2 << 16);
  std::uniform_int_distribution<char> uniformChar;
  std::mt19937 r(seed);

  // test for 16 times
  for (int times = 0; times < 16; ++times) {
    buffer->Clear();
    // random strings
    std::vector<std::string> randomStrings;
    randomStrings.resize(1024);
    for (auto &str : randomStrings) {
      std::uint32_t size = uniform(r);
      str.resize(size, '\0');
      for (int index = 0; index < size; ++index) {
        str[index] = uniformChar(r);
      }
      codeC->Encode(buffer.get(), str);
      ASSERT_EQ(codeC->Decode(buffer.get()), str);
    }
  }
}

TEST(test_codeC, test_codeC2) {
  clsn::RingBuffer *buffer = new clsn::RingBuffer();
  //  std::unique_ptr<> buffer = std::make_unique<>();
  std::unique_ptr<clsn::CodeC> codeC = std::make_unique<clsn::DefaultCodeC>();

  std::random_device randomDevice;
  unsigned int seed = randomDevice();  // 生成一个随机的种子值
  std::uniform_int_distribution<std::uint32_t> uniform(2, 2 << 16);
  std::uniform_int_distribution<char> uniformChar;
  std::mt19937 r(seed);

  // test for 16 times
  for (int times = 0; times < 16; ++times) {
    buffer->Clear();
    // random strings
    std::vector<std::string> randomStrings;
    randomStrings.resize(1024);
    for (int i = 0; i < randomStrings.size(); ++i) {
      std::string &str = randomStrings[i];
      std::uint32_t size = uniform(r);
      str.resize(size, '\0');
      for (int index = 0; index < size; ++index) {
        str[index] = uniformChar(r);
      }
      codeC->Encode(buffer, str);
      ASSERT_EQ(str, buffer->CPeek(str.size())) << "readStr and randomStr unequal";
    }
    for (auto &str : randomStrings) {
      ASSERT_EQ(codeC->Decode(buffer), str);
    }
  }
  delete buffer;
}

TEST(test_codeC, test_codeC3) {
  clsn::RingBuffer *buffer = new clsn::RingBuffer();
  //  std::unique_ptr<> buffer = std::make_unique<>();
  std::unique_ptr<clsn::CodeC> codeC = std::make_unique<clsn::DefaultCodeC>();

  std::random_device randomDevice;
  unsigned int seed = randomDevice();  // 生成一个随机的种子值
  std::uniform_int_distribution<std::uint32_t> uniform(2, 2 << 16);
  std::uniform_int_distribution<char> uniformChar;
  std::mt19937 r(seed);

  // test for 16 times
  for (int times = 0; times < 16; ++times) {
    buffer->Clear();
    // random strings
    std::vector<std::string> randomStrings;
    randomStrings.resize(1024);
    for (int i = 0; i < randomStrings.size(); ++i) {
      std::string &str = randomStrings[i];
      std::uint32_t size = uniform(r);
      str.resize(size, '\0');
      for (int index = 0; index < size; ++index) {
        str[index] = uniformChar(r);
      }
    }
    //    codeC->Encode(buffer, str);
    //    ASSERT_EQ(str, buffer->CPeek(str.size())) << "readStr and randomStr unequal";
    std::uniform_int_distribution<std::uint32_t> uniformStrings(1, 1024);
    std::uint32_t writeIndex = 0;
    std::uint32_t readIndex = 0;
    std::uint32_t totalWrite = 0;
    std::uint32_t totalRead = 0;
    do {
      writeIndex = uniformStrings(r);
      for (std::uint32_t i = totalWrite; i < totalWrite + writeIndex && i < randomStrings.size(); ++i) {
        codeC->Encode(buffer, randomStrings[i]);
      }
      totalWrite += writeIndex;

      readIndex = uniformStrings(r);
      if (readIndex > totalWrite - totalRead) {
        readIndex = totalWrite - totalRead;
      }
      for (std::uint32_t i = totalRead; i < totalRead + readIndex && i < randomStrings.size(); ++i) {
        ASSERT_EQ(codeC->Decode(buffer), randomStrings[i]);
      }
      totalRead += readIndex;
    } while (readIndex < randomStrings.size() || writeIndex < randomStrings.size());
  }
}