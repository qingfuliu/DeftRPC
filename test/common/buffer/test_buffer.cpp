//
// Created by qingfuliu on 1/20/25.
//
#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>
#include "common/buffer/RingBuffer.h"

TEST(test_buffer, test_write_read) {
  std::unique_ptr<clsn::Buffer> buffer = std::make_unique<clsn::RingBuffer>();

  std::random_device randomDevice;
  unsigned int seed = randomDevice();  // 生成一个随机的种子值
  std::uniform_int_distribution<std::uint32_t> uniform(2, 2 << 16);
  std::mt19937 r(seed);
  std::uniform_int_distribution<char> uniformChar;
  // test for 1024 times
  for (int times = 0; times < 1024; ++times) {
    // random string
    std::uint32_t size = uniform(r);
    std::string randomStr(size, '\0');
    for (int index = 0; index < size; ++index) {
      randomStr[index] = uniformChar(r);
    }
    // test for Write and ReadAll
    {
      buffer->Write(randomStr);
      ASSERT_EQ(buffer->Size(), randomStr.size()) << "buffer and randomStr are of unequal length";
      std::string readStr = buffer->ReadAll();
      ASSERT_EQ(readStr, randomStr) << "readStr and randomStr unequal";
    }
    // test for Write and Read
    {
      buffer->Write(randomStr);
      std::string readStr;
      std::uniform_int_distribution<std::uint32_t> uniformSize(1, randomStr.size());
      std::uint32_t remaining = randomStr.size();
      do {
        std::uint32_t section = uniformSize(r);
        if (section > remaining) {
          section = remaining;
        }
        remaining -= section;
        //...... [section][remaining]
        std::uint32_t begin = randomStr.size() - remaining - section;
        std::string readStr = buffer->Read(section);
        ASSERT_EQ(readStr.size(), section) << "readStr's size and section unequal";
        ASSERT_EQ(readStr, randomStr.substr(begin, section)) << "readStr and randomStr unequal";
        ASSERT_EQ(buffer->Size(), remaining) << "buffer and randomStr are of unequal length";
      } while (remaining > 0);
    }
  }
}

TEST(test_buffer, test_write_peek) {
  std::unique_ptr<clsn::Buffer> buffer = std::make_unique<clsn::RingBuffer>();

  std::random_device randomDevice;
  unsigned int seed = randomDevice();  // 生成一个随机的种子值
  std::uniform_int_distribution<std::uint32_t> uniform(2, 2 << 16);
  std::mt19937 r(seed);
  std::uniform_int_distribution<char> uniformChar;
  // test for 1024 times
  for (int times = 0; times < 1024; ++times) {
    // random string
    buffer->Clear();
    std::uint32_t size = uniform(r);
    std::string randomStr(size, '\0');
    for (int index = 0; index < size; ++index) {
      randomStr[index] = uniformChar(r);
    }
    // test for Write and Peek
    {
      buffer->Write(randomStr);
      std::uniform_int_distribution<std::uint32_t> uniformSize(1, randomStr.size());
      // peek 1024 times
      for (int timesPeek = 0; timesPeek < 1024; ++timesPeek) {
        std::uint32_t peekSize = uniformSize(r);
        std::string readStr = buffer->Peek(peekSize);
        ASSERT_EQ(readStr, randomStr.substr(0, peekSize)) << "readStr and randomStr unequal";
        ASSERT_EQ(buffer->Size(), randomStr.size()) << "buffer and randomStr are of unequal length";
      }
    }
  }
}

TEST(test_buffer, test_buffer) {
  std::unique_ptr<clsn::Buffer> buffer = std::make_unique<clsn::RingBuffer>();

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
    }
    std::vector<std::string> randomStringsForRead = randomStrings;
    // test for Write,Read,Peek
    do {
      // write
      std::string writeString;
      std::uint64_t bufferSize = buffer->Size();
      std::uint32_t writeSize = uniform(r);
      std::uint32_t writeSizeSave = writeSize;
      for (std::uint32_t index = 0; index < randomStrings.size() && writeSize > 0; ++index) {
        if (randomStrings[index].size() >= writeSize) {
          writeString += randomStrings[index].substr(0, writeSize);
          buffer->Write(randomStrings[index].substr(0, writeSize));

          ASSERT_EQ(randomStrings[index].substr(0, writeSize), buffer->CPeek(writeSize))
              << "readStr and randomStr unequal";
          ASSERT_EQ(writeString, buffer->CPeek(writeString.size())) << "readStr and randomStr unequal";
          randomStrings[index] = randomStrings[index].substr(writeSize, randomStrings[index].size() - writeSize);
          writeSize = 0;
          break;
        }
        writeString += randomStrings[index];
        buffer->Write(randomStrings[index]);
        ASSERT_EQ(randomStrings[index], buffer->CPeek(randomStrings[index].size())) << "readStr and randomStr unequal";
        ASSERT_EQ(writeString, buffer->CPeek(writeString.size())) << "readStr and randomStr unequal";

        writeSize -= randomStrings[index].size();
        randomStrings[index].clear();
      }

      ASSERT_EQ(writeSizeSave + bufferSize - writeSize, buffer->Size()) << "readStr and randomStr unequal";

      ASSERT_EQ(writeString, buffer->CPeek(writeString.size())) << "readStr and randomStr unequal";
      while (!randomStrings.empty() && randomStrings.begin()->empty()) {
        randomStrings.erase(randomStrings.begin());
      }
      // read
      std::uint32_t readSize = uniform(r);
      if (readSize > buffer->Size()) {
        readSize = buffer->Size();
      }
      std::string needleReadString;
      for (std::uint32_t index = 0; index < randomStringsForRead.size() && readSize > 0; ++index) {
        if (randomStringsForRead[index].size() >= readSize) {
          needleReadString += randomStringsForRead[index].substr(0, readSize);
          randomStringsForRead[index] =
              randomStringsForRead[index].substr(readSize, randomStringsForRead[index].size() - readSize);
          readSize = 0;
          break;
        }
        needleReadString += randomStringsForRead[index];
        readSize -= randomStringsForRead[index].size();
        randomStringsForRead[index].clear();
      }
      while (!randomStringsForRead.empty() && randomStringsForRead.begin()->empty()) {
        randomStringsForRead.erase(randomStringsForRead.begin());
      }

      std::string peekString = buffer->Peek(needleReadString.size());
      ASSERT_EQ(peekString.size(), needleReadString.size()) << "readStr and randomStr unequal";
      ASSERT_EQ(peekString, needleReadString) << "readStr and randomStr unequal";

      std::string readString = buffer->Read(needleReadString.size());
      ASSERT_EQ(readString.size(), needleReadString.size()) << "readStr and randomStr unequal";
      ASSERT_EQ(readString, needleReadString) << "readStr and randomStr unequal";
    } while (!randomStrings.empty() && !randomStringsForRead.empty());
  }
}