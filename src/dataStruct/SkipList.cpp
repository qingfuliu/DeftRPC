//
// Created by lqf on 23-5-15.
//

#include "dataStruct/SkipList.h"
#include "log/Log.h"

namespace clsn {
std::pair<SkipListNode *, SizeType> SkipList::Insert(ScoreType score, const std::string &val) noexcept {
  SkipLevelType lv = GetRandomLevel();
  auto node = new SkipListNode(score, val, lv);

  int size = lv >= m_level_ ? lv + 1 : m_level_;
  std::vector<SkipListNode *> update(size, nullptr);
  std::vector<SizeType> rank(size);
  GetUpdateVec(update, rank, score, val, size);

  for (int i = m_level_ - 1; i > lv; --i) {
    update[i]->SetSpan(i, update[i]->GetSpan(i) + 1);
  }

  for (int i = lv; i >= 0; --i) {
    auto temp = update[i]->SetNext(i, node);
    node->SetNext(i, temp);

    node->SetSpan(i, update[i]->GetSpan(i) - (rank[0] - rank[i]));
    update[i]->SetSpan(i, 1 + (rank[0] - rank[i]));
  }

  if (m_level_ <= lv) {
    m_level_ = lv + 1;
  }

  node->SetPrev(update[0]);
  if (node->GetNext(0) != nullptr) {
    node->GetNext(0)->SetPrev(node);
  } else {
    m_tail_ = node;
  }
  ++m_length_;
  return {node, rank[0] + 1};
}

bool SkipList::Delete(ScoreType score, const std::string &val) noexcept {
  std::vector<SkipListNode *> update(m_level_, nullptr);
  std::vector<SizeType> rank(m_level_, 0);
  GetUpdateVec(update, rank, score, val, m_level_);

  SkipListNode *node = update[0]->GetNext(0);
  if (node != nullptr && node->GetScore() == score && node->GetVal() == val) {
    int i = m_level_ - 1;
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
    for (i = m_level_; i >= 0; --i) {
      if (m_head_->GetNext(i) == nullptr) {
        --m_level_;
      } else {
        break;
      }
    }
    --m_length_;
    delete node;
    return true;
  }
  return false;
}

std::pair<SkipListNode *, SizeType> SkipList::Find(ScoreType score, const std::string &val) noexcept {
  std::vector<SkipListNode *> update(m_level_, nullptr);
  std::vector<SizeType> rank(m_level_, 0);
  GetUpdateVec(update, rank, score, val, m_level_);
  for (int i = m_level_ - 1; i >= 0; --i) {
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
 * @param update 需要更新 m_next_ 的节点
 * @param rank   update[i]的排名
 * @param score
 * @param val
 * @param mLevel
 */
void SkipList::GetUpdateVec(std::vector<SkipListNode *> &update, std::vector<SizeType> &rank, ScoreType score,
                            const std::string &val, SkipLevelType mLevel) noexcept {
  assert(mLevel <= update.size());
  SkipListNode *header = m_head_.get();
  for (int i = mLevel - 1; i >= 0; --i) {
    if (i >= m_level_) {
      rank[i] = 0;
      header->SetSpan(i, m_length_);
    } else if (i == m_level_ - 1 || i == mLevel - 1) {
      rank[i] = 0;
    } else if (rank[i] < m_level_) {
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