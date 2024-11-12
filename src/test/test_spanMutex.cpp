//
// Created by lqf on 23-4-21.
//
#include <gtest/gtest.h>
#include <iostream>
#include "common/SpanMutex.h"

static int a = 0;
static clsn::SpanMutex spanM;

void test_spanMutex() {
  {
    spanM.Lock();
    //        std::cout<<"Lock"<<std::endl;
    ++a;
    spanM.UnLock();
  }
}

void test_spanMutex_Reentrant() {
  clsn::ReentrantMutex<clsn::SpanMutex> mutexRe{};
  {
    mutexRe.Lock();
    std::cout << clsn::Thread::thisThreadId() << std::endl;
    std::cout << "Lock" << std::endl;
    mutexRe.Lock();
    std::cout << "Lock1" << std::endl;
    mutexRe.Lock();
    std::cout << "Lock2" << std::endl;
    ++a;
    spanM.UnLock();
  }
}

void test_spanMutex_res() {
  { std::cout << a << std::endl; }
}

TEST(test_spanMutex, test_spanMutex) {}
