//
// Created by lqf on 23-4-18.
//

#include <gtest/gtest.h>
#include <iostream>
#include <random>
#include "coroutine/Coroutine.h"

TEST(test_coroutine, testSwapInOneLayer) {
  std::random_device randomDevice;
  unsigned int seed = randomDevice();
  std::uniform_int_distribution<int> uniform(2, 2 << 16);
  std::uniform_int_distribution<char> uniformChar;
  std::mt19937 r(seed);

  auto cur = clsn::CreateCoroutine(nullptr, nullptr, true);
  auto next = clsn::CreateCoroutine(nullptr, nullptr);
  int flag = 0;
  std::vector<int> v_inner;
  std::vector<int> v_outer;

  int loop_times = uniform(r);
  v_inner.reserve(loop_times);
  v_outer.reserve(loop_times);

  auto func = [&cur, &next, &flag, &v_inner, loop_times]() {
    while (flag != loop_times) {
      v_inner.push_back(flag);
      cur->SwapIn(*next);
    }
    cur->SwapIn(*next);
  };
  next->Reset(func);

  for (int i = 0; i < loop_times; ++i) {
    v_outer.push_back(i);
    next->SwapIn(*cur);
    ++flag;
  }
  ASSERT_EQ(v_inner, v_outer);
}

TEST(test_coroutine, testSwapMultiLayer) {
  auto cur = clsn::CreateCoroutine(nullptr, nullptr, true);

  std::random_device randomDevice;
  unsigned int seed = randomDevice();
  std::uniform_int_distribution<int> uniform(2, 2 << 16);
  std::uniform_int_distribution<char> uniformChar;
  std::mt19937 r(seed);
  std::vector<int> v_inner;
  std::vector<int> v_outer;
  int flag = 0;
  int loop_times = 2;

  std::vector<std::unique_ptr<clsn::Coroutine>> coroutines(loop_times);
  for (auto &coroutine : coroutines) {
    coroutine = clsn::CreateCoroutine(nullptr, nullptr);
  }

  for (size_t i = 0; i < coroutines.size(); ++i) {
    auto func = [&cur, &coroutines, i, loop_times, &v_inner, &flag]() {
      v_inner.push_back(flag);
      ++flag;
      if (i < loop_times - 1) {
        coroutines[i + 1]->SwapIn(*coroutines[i]);
      }
      if (i > 0) {
        coroutines[i - 1]->SwapIn(*coroutines[i]);
      } else {
        cur->SwapIn(*coroutines[0]);
      }
    };
    coroutines[i]->Reset(func);
  }
  coroutines[0]->SwapIn(*cur);

  v_outer.reserve(loop_times);
  for (int i = 0; i < loop_times; ++i) {
    v_outer.push_back(i);
  }
  ASSERT_EQ(flag, loop_times);
  ASSERT_EQ(v_inner, v_outer);
}