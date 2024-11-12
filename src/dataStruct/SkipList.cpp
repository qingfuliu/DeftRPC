//
// Created by lqf on 23-5-15.
//

#include "dataStruct/SkipList.h"
#include "log/Log.h"

namespace clsn {
std::pair<SkipListNode *, SizeType> SkipList::Insert(ScoreType score, const std::string &val) noexcept {
  SkipLevelType lv = GetRandomLevel();
  auto node = new SkipListNode(score, val, lv);

  int size = lv >= level ? lv + 1 : level;
  std::vector<SkipListNode *> update(size, nullptr);
  std::vector<SizeType> rank(size);
  getUpdateVec(update, rank, score, val, size);

  for (int i = level - 1; i > lv; --i) {
    update[i]->SetSpan(i, update[i]->GetSpan(i) + 1);
  }

  for (int i = lv; i >= 0; --i) {
    auto temp = update[i]->SetNext(i, node);
    node->SetNext(i, temp);

    node->SetSpan(i, update[i]->GetSpan(i) - (rank[0] - rank[i]));
    update[i]->SetSpan(i, 1 + (rank[0] - rank[i]));
  }

  if (level <= lv) {
    level = lv + 1;
  }

  node->SetPrev(update[0]);
  if (node->GetNext(0) != nullptr) {
    node->GetNext(0)->SetPrev(node);
  } else {
    tail = node;
  }
  ++length;
  return {node, rank[0] + 1};
}

bool SkipList::Delete(ScoreType score, const std::string &val) noexcept {
  std::vector<SkipListNode *> update(level, nullptr);
  std::vector<SizeType> rank(level, 0);
  getUpdateVec(update, rank, score, val, level);

  SkipListNode *node = update[0]->GetNext(0);
  if (node != nullptr && node->GetScore() == score && node->GetVal() == val) {
    int i = level - 1;
    for (; i >= 0; --i) {
      if (update[i]->GetNext(i) == node) {
        break;
      }
      update[i]->SetSpan(i, update[i]->GetSpan(i) - 1);
    }

    for (; i >= 0; --i) {
      update[i]->SetNext(i, node->GetNext(i));
      node->SetNext(i, nullptr);

      update[i]->SetSpan(i, update[i]->GetSpan(i) + node->GetSpan(i) - 1);
    }
    for (i = level; i >= 0; --i) {
      if (head->GetNext(i) == nullptr) {
        --level;
      } else {
        break;
      }
    }
    --length;
    delete node;
    return true;
  }
  return false;
}

std::pair<SkipListNode *, SizeType> SkipList::Find(ScoreType score, const std::string &val) noexcept {
  std::vector<SkipListNode *> update(level, nullptr);
  std::vector<SizeType> rank(level, 0);
  getUpdateVec(update, rank, score, val, level);
  for (int i = level - 1; i >= 0; --i) {
    auto next = update[i]->GetNext(i);
    if (next != nullptr && next->GetScore() == score && next->GetVal() == val) {
      return {next, rank[i] + update[i]->GetSpan(i)};
    }
  }
  return {nullptr, 0};
}

std::pair<SkipListNode *, SizeType> SkipList::Modify(ScoreType score, const std::string &val) noexcept {
  if (!Delete(score, val)) {
    return {nullptr, 0};
  }
  return Insert(score, val);
}

/**
 *
 * @param update 需要更新 next 的节点
 * @param rank   update[i]的排名
 * @param score
 * @param val
 * @param mLevel
 */
void SkipList::getUpdateVec(std::vector<SkipListNode *> &update, std::vector<SizeType> &rank, ScoreType score,
                            const std::string &val, SkipLevelType mLevel) noexcept {
  assert(mLevel <= update.size());
  SkipListNode *header = head.get();
  for (int i = mLevel - 1; i >= 0; --i) {
    if (i >= level) {
      rank[i] = 0;
      header->SetSpan(i, length);
    } else if (i == level - 1 || i == mLevel - 1) {
      rank[i] = 0;
    } else if (rank[i] < level) {
      rank[i] = rank[i + 1];
    }
    while (header->GetNext(i) != nullptr &&
           (header->GetNext(i)->GetScore() < score ||
            header->GetNext(i)->GetScore() == score && header->GetNext(i)->GetVal() < val)) {
      rank[i] += header->GetSpan(i);
      header = header->GetNext(i);
    }
    update[i] = header;
  }
}

}  // namespace clsn