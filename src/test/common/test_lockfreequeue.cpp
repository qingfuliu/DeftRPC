//
// Created by lqf on 23-5-6.
//
#include <gtest/gtest.h>
#include <random>
#include <thread>
#include <unordered_set>
#include <vector>
#include "common/LockFreeQueue.h"

using namespace clsn;

TEST(test_lockFreeQueue, testEnQueue) {
  // simple
  {
    std::vector<std::thread> ts(100);
    LockFreeQueue<int> a;
    std::unordered_set<int> set;
    for (int i = 0; i < 100; i++) {
      set.insert(i);
    }

    for (int i = 0; i < 100; i++) {
      ts[i] = std::thread([=, &a]() -> void { a.EnQueue(i); });
    }
    for (int i = 0; i < 100; i++) {
      ts[i].join();
    }

    ASSERT_EQ(a.Size(), set.size());
    while (!a.Empty()) {
      int i = a.Dequeue();
      ASSERT_EQ(set.count(i), 1);
      set.erase(i);
    }
    ASSERT_EQ(a.Size(), set.size());
  }

  // random
  {
    std::vector<std::thread> ts(512);
    LockFreeQueue<int> a;
    std::multiset<int> set;
    std::vector<std::vector<int>> srcs(512, std::vector(0, 0));
    std::mutex m;

    std::random_device randomDevice;
    unsigned int seed = randomDevice();  // 生成一个随机的种子值
    std::uniform_int_distribution<int> uniform(2, 100);
    std::uniform_int_distribution<char> uniformChar;
    std::mt19937 r(seed);
    int64_t total_size = 0;
    for (int i = 0; i < 512; i++) {
      std::uint32_t size = uniform(r);
      total_size += size;
      while (size != 0) {
        --size;
        int k = uniform(r);
        srcs[i].push_back(k);
        set.insert(k);
      }
    }

    for (int i = 0; i < 512; i++) {
      ts[i] = std::thread([=, &a, &srcs]() -> void {
        for (auto k : srcs[i]) {
          a.EnQueue(k);
        }
      });
    }

    for (int i = 0; i < 512; i++) {
      ts[i].join();
    }

    ASSERT_EQ(a.Size(), total_size);
    ASSERT_EQ(set.size(), total_size);
    while (!a.Empty()) {
      int i = a.Dequeue();
      ASSERT_NE(set.count(i), 0);
      auto it = set.find(i);
      set.erase(it);
    }
    ASSERT_EQ(a.Size(), set.size());
  }
}