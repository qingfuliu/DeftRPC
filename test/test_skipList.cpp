//
// Created by lqf on 23-5-15.
//

#include <cassert>
#include <tuple>
#include <vector>
#include "dataStruct/SkipList.h"
#include "log/Log.h"

int main() {
  clsn::Init<0>({clsn::CreateConsoleLogAppender("[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s", clsn::LogLevel::Debug)});
  clsn::SkipList list;
  std::vector<std::tuple<clsn::ScoreType, std::string, clsn::SizeType>> t;

  for (int i = 0; i < 1000; ++i) {
    CLSN_LOG_DEBUG << i;
    if ((i & 1) == 0) {  // 奇数
      t.emplace_back(i, "lqf", i);
      auto [_, k] = list.Insert(std::get<0>(t.back()), std::get<1>(t.back()));
      assert(k == i + 1);
    } else {
      t.emplace_back(i - 1, "zzz", i);
      auto [_, k] = list.Insert(std::get<0>(t.back()), std::get<1>(t.back()));
      assert(k == i + 1);
    }
  }

  for (int i = 0; i < 1000; ++i) {
    auto [p, rank] = list.Find(std::get<0>(t[i]), std::get<1>(t[i]));
    assert(p != nullptr);
    assert(p->GetVal() == std::get<1>(t[i]));
    assert(rank - 1 == std::get<2>(t[i]));
  }

  int size = list.GetSize();

  for (int i = 0; i < 250; ++i) {
    assert(list.GetSize() == size--);
    if ((i & 1) == 0) {  // 奇数
      assert(list.Delete(i, "lqf"));
    } else {
      assert(list.Delete(i - 1, "zzz"));
    }
  }

  for (int i = 250; i < 1000; ++i) {
    auto [p, rank] = list.Find(std::get<0>(t[i]), std::get<1>(t[i]));
    assert(p != nullptr);
    assert(p->GetVal() == std::get<1>(t[i]));
    assert(rank - 1 == std::get<2>(t[i]) - 250);
  }

  for (int i = 250; i < 500; ++i) {
    assert(list.GetSize() == size--);

    if ((i & 1) == 0) {  // 奇数
      assert(list.Delete(i, "lqf"));
    } else {
      assert(list.Delete(i - 1, "zzz"));
    }
  }

  for (int i = 500; i < 1000; ++i) {
    auto [p, rank] = list.Find(std::get<0>(t[i]), std::get<1>(t[i]));
    assert(p != nullptr);
    assert(p->GetVal() == std::get<1>(t[i]));
    assert(rank - 1 == std::get<2>(t[i]) - 500);
  }

  for (int i = 1000 - 1; i > 750; --i) {
    assert(list.GetSize() == size--);

    if ((i & 1) == 0) {  // 奇数
      assert(list.Delete(i, "lqf"));
    } else {
      assert(list.Delete(i - 1, "zzz"));
    }
  }

  for (int i = 500; i <= 750; ++i) {
    auto [p, rank] = list.Find(std::get<0>(t[i]), std::get<1>(t[i]));
    assert(p != nullptr);
    assert(p->GetVal() == std::get<1>(t[i]));
    assert(rank - 1 == std::get<2>(t[i]) - 500);
  }

  for (int i = 750; i >= 500; --i) {
    assert(list.GetSize() == size--);
    if ((i & 1) == 0) {  // 奇数
      assert(list.Delete(i, "lqf"));
    } else {
      assert(list.Delete(i - 1, "zzz"));
    }
  }
}