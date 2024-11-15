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

namespace clsn {

    using ScoreType = uint32_t;
    using SkipLevelType = uint8_t;
    using SizeType = uint32_t;
//    using SkipRankType = uint32_t;

    constexpr SkipLevelType MAX_LEVEL = UINT8_MAX;
    constexpr double LEVEL_RADIO = 0.25;

    class SkipListNode;
    namespace detail {
        class SkipListLevel {
        public:
            SkipListNode *m_next_{nullptr};
            SizeType m_span_{0};
        };
    }  // namespace detail

    class SkipListNode {
    public:
        SkipListNode() noexcept: SkipListNode(-1, "", MAX_LEVEL) {}

        SkipListNode(ScoreType s, std::string v, SkipLevelType lv) noexcept
                : m_score_(s), m_val_(std::move(v)),
                  m_level_(std::make_unique<detail::SkipListLevel[]>(lv + 1)) {
            for (int i = 0; i < lv; i++) {
                m_level_[i].m_next_ = nullptr;
                m_level_[i].m_span_ = 0;
            }
        }

        ~SkipListNode() = default;

        SkipListNode *GetNext(SkipLevelType i) noexcept { return m_level_[i].m_next_; }

        SizeType GetSpan(SkipLevelType i) noexcept { return m_level_[i].m_span_; }

        void SetSpan(SkipLevelType i, SizeType s) noexcept { m_level_[i].m_span_ = s; }

        [[nodiscard]] const SkipListNode *GetNext(SkipLevelType i) const noexcept { return m_level_[i].m_next_; }

        SkipListNode *SetNext(SkipLevelType lv, SkipListNode *n) noexcept {
            assert(this != n);
            auto prev = m_level_[lv].m_next_;
            m_level_[lv].m_next_ = n;

            if (lv == 0) {
                auto temp = m_next_.release();
                assert(temp == prev);
                m_next_.reset(n);
            }
            return prev;
        }

        void SetPrev(SkipListNode *p) noexcept { m_prev_ = p; }

        [[nodiscard]] const std::string &GetVal() const noexcept { return m_val_; }

        std::string &GetVal() noexcept { return m_val_; }

        [[nodiscard]] ScoreType GetScore() const noexcept { return m_score_; }

        bool operator<(const SkipListNode *node) const noexcept {
            return node == nullptr || (node->m_score_ > m_score_) ||
                   (node->m_score_ == m_score_ && node->m_val_ > m_val_);
        }

    private:
        ScoreType m_score_{0};
        std::string m_val_{};
        SkipListNode *m_prev_{nullptr};
        std::unique_ptr<SkipListNode> m_next_{nullptr};
        std::unique_ptr<detail::SkipListLevel[]> m_level_;
    };

    class SkipList {
    public:
        SkipList() noexcept = default;

        ~SkipList() = default;

        [[nodiscard]] SizeType GetSize() const noexcept { return m_length_; }

        std::pair<SkipListNode *, SizeType> Insert(ScoreType score, const std::string &val) noexcept;

        bool Delete(ScoreType score, const std::string &val) noexcept;

        std::pair<SkipListNode *, SizeType> Find(ScoreType score, const std::string &val) noexcept;

        std::pair<SkipListNode *, SizeType> Modify(ScoreType score, const std::string &val) noexcept;

    private:
        static inline SkipLevelType GetRandomLevel() noexcept {
            SkipLevelType res = 0;
            while ((static_cast<unsigned int>(random()) & MAX_LEVEL) < (MAX_LEVEL >> 2)) {
                ++res;
            }
            res = res > MAX_LEVEL ? MAX_LEVEL : res;
            return res;
        }

        void GetUpdateVec(std::vector<SkipListNode *> &update, std::vector<SizeType> &rank, ScoreType score,
                          const std::string &val, SkipLevelType mLevel) noexcept;

    private:
        SizeType m_length_{0};
        SkipLevelType m_level_{1};
        std::unique_ptr<SkipListNode> m_head_{std::make_unique<SkipListNode>()};
        SkipListNode *m_tail_{m_head_.get()};
    };

}  // namespace clsn

#endif  // DEFTRPC_SKIPLIST_H
