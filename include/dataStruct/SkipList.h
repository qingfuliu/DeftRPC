//
// Created by lqf on 23-5-15.
//

#ifndef DEFTRPC_SKIPLIST_H
#define DEFTRPC_SKIPLIST_H

#include <cassert>
#include <cstdint>
#include <limits>
#include <memory>
#include <random>
#include <vector>

namespace CLSN {

using ScoreType = uint32_t;
using SkipLevelType = uint8_t;
using SizeType = uint32_t;
//    using SkipRankType = uint32_t;

constexpr SkipLevelType MaxLevel = UINT8_MAX;
constexpr double LevelRadio = 0.25;

class SkipListNode;
namespace detail {
class SkipListLevel {
 public:
  SkipListNode *next{nullptr};
  SizeType span{0};
};
}  // namespace detail

class SkipListNode {
 public:
  SkipListNode() noexcept : SkipListNode(-1, "", MaxLevel) {}

  SkipListNode(ScoreType s, std::string v, SkipLevelType lv) noexcept
      : score(s), val(std::move(v)), prev(nullptr), level(std::make_unique<detail::SkipListLevel[]>(lv + 1)) {
    for (int i = 0; i < lv; i++) {
      level[i].next = nullptr;
      level[i].span = 0;
    }
  }

  ~SkipListNode() = default;

  SkipListNode *GetNext(SkipLevelType i) noexcept { return level[i].next; }

  SizeType GetSpan(SkipLevelType i) noexcept { return level[i].span; }

  void SetSpan(SkipLevelType i, SizeType s) noexcept { level[i].span = s; }

  [[nodiscard]] const SkipListNode *GetNext(SkipLevelType i) const noexcept { return level[i].next; }

  SkipListNode *SetNext(SkipLevelType lv, SkipListNode *n) noexcept {
    assert(this != n);
    auto prev = level[lv].next;
    level[lv].next = n;

    if (lv == 0) {
      auto temp = next.release();
      assert(temp == prev);
      next.reset(n);
    }
    return prev;
  }

  void SetPrev(SkipListNode *p) noexcept { prev = p; }

  [[nodiscard]] const std::string &GetVal() const noexcept { return val; }

  std::string &GetVal() noexcept { return val; }

  [[nodiscard]] ScoreType GetScore() const noexcept { return score; }

  bool operator<(const SkipListNode *node) const noexcept {
    return node == nullptr || (node->score > score) || (node->score == score && node->val > val);
  }

 private:
  ScoreType score{0};
  std::string val{};
  SkipListNode *prev{nullptr};
  std::unique_ptr<SkipListNode> next{nullptr};
  std::unique_ptr<detail::SkipListLevel[]> level;
};

class SkipList {
 public:
  SkipList() noexcept : level(1), length(0), head(std::make_unique<SkipListNode>()), tail(head.get()) {}

  ~SkipList() = default;

  [[nodiscard]] SizeType GetSize() const noexcept { return length; }

  std::pair<SkipListNode *, SizeType> Insert(ScoreType score, const std::string &val) noexcept;

  bool Delete(ScoreType score, const std::string &val) noexcept;

  std::pair<SkipListNode *, SizeType> Find(ScoreType score, const std::string &val) noexcept;

  std::pair<SkipListNode *, SizeType> Modify(ScoreType score, const std::string &val) noexcept;

 private:
  static inline SkipLevelType GetRandomLevel() noexcept {
    SkipLevelType res = 0;
    while ((static_cast<unsigned int>(random()) & MaxLevel) < (MaxLevel >> 2)) {
      ++res;
    }
    res = res > MaxLevel ? MaxLevel : res;
    return res;
  }

  void getUpdateVec(std::vector<SkipListNode *> &update, std::vector<SizeType> &rank, ScoreType score,
                    const std::string &val, SkipLevelType mLevel) noexcept;

 private:
  SizeType length;
  SkipLevelType level;
  std::unique_ptr<SkipListNode> head;
  SkipListNode *tail;
};

}  // namespace CLSN

#endif  // DEFTRPC_SKIPLIST_H
