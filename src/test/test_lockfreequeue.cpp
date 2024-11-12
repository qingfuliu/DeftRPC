//
// Created by lqf on 23-5-6.
//
#include <benchmark/benchmark.h>
#include <iostream>
#include <thread>
#include "common/LockFreeQueue.h"

using namespace clsn;
static void LockFreeQueueTest() {
  LockFreeQueue<int> a;
  std::vector<std::thread> ts(100);

  for (int i = 0; i < 100; i++) {
    ts[i] = std::thread([&a, i]() -> void { a.EnQueue(i); });
  }

  for (int i = 0; i < 100; i++) {
    ts[i].join();
  }

  std::cout << a.Size() << std::endl;

  for (int i = 0; i < 90; i++) {
    int temp = i;
    ts[i] = std::thread([&a, temp]() -> void { std::cout << a.Dequeue() << std::endl; });
  }

  for (int i = 0; i < 90; i++) {
    ts[i].join();
  }
}

int main() { LockFreeQueueTest(); }
// BENCHMARK(LockFreeQueueTest);
// BENCHMARK_MAIN();