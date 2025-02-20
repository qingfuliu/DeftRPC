//
// Created by lqf on 23-5-14.
//
#include "common/Iterator.h"
#include "dataStruct/Hash.h"
#include "log/Log.h"

#include <gtest/gtest.h>

using namespace clsn;

struct TestEntry {
  int a;
  int b;
};

void test_HashTableIterator(HashTable *hash) {
  std::map<int, int> p;

  auto it = hash->GetIterator();
  {
    void *prevVal = nullptr;
    int i = 0;
    std::unordered_set<std::string> s;
    while (it->IsValid()) {
      //            if (i != 0) {
      //                it->Prev();
      ////                auto val = HashTable::GetValByIterator(it.get());
      ////                ASSERT_TRUE(val == prevVal);
      //                it->Next();
      //            }
      auto key = HashTable::GetKeyByIterator(it.get());
      auto val = HashTable::GetValByIterator(it.get());
      prevVal = val;
      p[i] = *static_cast<int *>(val);

      ASSERT_TRUE(s.find(key) == s.end());
      s.insert(key);

      ++i;
      CLSN_LOG_DEBUG << i << " " << key << " " << *static_cast<int *>(val) << " ";
      it->Next();
    }
    CLSN_LOG_DEBUG << i;
  }

  {
    it->Reset();
    int i = 0;
    while (it->IsValid()) {
      auto key = HashTable::GetKeyByIterator(it.get());
      auto val = HashTable::GetValByIterator(it.get());
      ASSERT_TRUE(p[i] == *static_cast<int *>(val));
      ++i;
      if (i == 5000) {
        break;
      }
      it->Next();
    }
  }

  {
    int y = 5000;
    std::unordered_set<std::string> s;
    while (it->IsValid()) {
      --y;

      auto key = HashTable::GetKeyByIterator(it.get());
      auto val = HashTable::GetValByIterator(it.get());
      CLSN_LOG_DEBUG << y << " " << key << " " << *static_cast<int *>(val) << " ";
      ASSERT_TRUE(p[y] == *static_cast<int *>(val));
      ASSERT_TRUE(s.find(key) == s.end());
      s.insert(key);
      if (y == 0) {
        int a = 1;
      }
      it->Prev();
    }
  }
  //
  //    CLSN_LOG_DEBUG << y;
};

TEST(tset_hash, test_hash) {
  clsn::Init<0>({clsn::CreateConsoleLogAppender("[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s", clsn::LogLevel::Debug)});
  HashTable t;
  std::vector<std::string> vec(5000);
  CLSN_LOG_DEBUG << vec.capacity() << " " << vec.size();
  for (int i = 0; i < vec.size(); i++) {
    vec[i] = "lqf" + std::to_string(i);
    auto res = t.Insert(vec[i], i);
    ASSERT_TRUE(i == *static_cast<int *>(res->GetVal()));
    ASSERT_TRUE(t.Size() == i + 1);

    res = t.FindByKey(vec[i]);
    ASSERT_TRUE(i == *static_cast<int *>(res->GetVal()));
  }

  test_HashTableIterator(&t);

  CLSN_LOG_DEBUG << vec.capacity() << " " << vec.size();
  for (int i = 0; i < vec.size(); i++) {
    ASSERT_TRUE(nullptr == t.Insert(vec[i], i));
    auto res = t.FindByKey(vec[i]);
    ASSERT_TRUE(i == *static_cast<int *>(res->GetVal()));
  }

  for (int i = 0; i < vec.size() / 2; i++) {
    t.Delete(vec[i]);
    ASSERT_TRUE(t.Size() == (vec.size() - i - 1));
    auto res = t.FindByKey(vec[i]);
    ASSERT_TRUE(res == nullptr);
  }
}