//
// Created by lqf on 23-4-18.
//

#include <benchmark/benchmark.h>
#include "coroutine/Coroutine.h"
#include "coroutine/Task.h"

#include <iostream>

thread_local auto sharedMem = clsn::MakeSharedStack(0);

void otherFunc1() { std::cout << "otherFunc1!!" << std::endl; }

void otherFunc() {
  int a = 1;
  int b = 10000;
  int p[20];
  for (int i = 0; i < 20; i++) {
    p[i] = i;
  }
  std::cout << "before otherFunc11!!" << std::endl;

  auto routine = clsn::CreateCoroutine();
  routine->SetTask(otherFunc1);
  routine->SwapIn();
  std::cout << "after otherFunc11!!" << std::endl;
  assert(a == 1);
  routine->Reset(otherFunc1);
  std::cout << "before otherFunc122!!" << std::endl;
  routine->SwapIn();
  std::cout << "after otherFunc122!!" << std::endl;
  assert(b == 10000);
  for (int i = 0; i < 20; i++) {
    assert(p[i] == i);
  }
}

static void test_SwapIn() {
  std::cout << "before test_SwapIn!!" << std::endl;
  auto routine = clsn::CreateCoroutine();
  routine->SetTask(otherFunc);
  routine->SwapIn();
  std::cout << "after test_SwapIn!!" << std::endl;
}

void otherFuncShared() {
  int a = 1;
  int b = 10000;
  int sa[60000];
  int sa1[60000];
  int sa2[60000];
  int p[20];
  for (int i = 0; i < 20; i++) {
    p[i] = i;
  }
  std::cout << "before otherFunc11!!" << std::endl;

  auto routine = clsn::CreateCoroutine();
  routine->SwapIn();
  std::cout << "after otherFunc11!!" << std::endl;
  assert(a == 1);
  routine->Reset(otherFunc1);
  std::cout << "before otherFunc122!!" << std::endl;
  routine->SwapIn();
  std::cout << "after otherFunc122!!" << std::endl;
  assert(b == 10000);
  for (int i = 0; i < 20; i++) {
    assert(p[i] == i);
  }
}

static void test_sharedPtr_SwapIn() {
  std::cout << "before test_SwapIn!!" << std::endl;
  auto routine = clsn::CreateCoroutine(otherFuncShared, sharedMem.get());
  routine->SwapIn();
  std::cout << "after test_SwapIn!!" << std::endl;
  std::cout << "success!!" << std::endl;
}

int main() {
  //    test_sharedPtr_SwapIn();
  test_SwapIn();
}
// BENCHMARK(test_SwapIn)->Iterations(1);
// BENCHMARK(test_sharedPtr_SwapIn)->Iterations(10)->Threads(10);
// BENCHMARK_MAIN();