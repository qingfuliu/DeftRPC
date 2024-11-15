//
// Created by lqf on 23-5-14.
//
#include "dataStruct/Hash.h"
#include <cassert>
#include <utility>
#include "dataStructConfig.h"

namespace clsn {

    HashEntryBase *HashTable::FindByKey(const char *key, HashLenType len) noexcept {
        HashEntryBase *p;
        auto bucket = FindBucketByKey(key, len, &p);
        if (bucket == nullptr) {
            return p;
        }
        return nullptr;
    }

    void HashTable::Delete(const char *key, HashLenType len) noexcept {
        uint64_t hash_key = m_hash_func_(key, len);
        if (IsReHashing()) {
            ReHash();
        }

        if (NeedReduce()) {
            ExpendOrReduce(ExpendSize());
        }

        std::unique_ptr<HashEntryBase *[]> *a[2]{&m_table_, &m_rehash_table_};
        for (int i = 0; i < 2; i++) {
            uint64_t cur_key = hash_key & GetKeyMask(m_size_[i]);

            HashEntryBase *entry = (*a[i]).get()[cur_key];
            HashEntryBase *prev = nullptr;
            while (entry != nullptr) {
                if (entry->GetKey() == key) {
                    --m_used_[i];
                    if (prev == nullptr) {
                        (*a[i]).get()[cur_key] = entry->GetNext();
                    } else {
                        prev->SetNext(entry->GetNext());
                    }
                    if (entry->GetNext() != nullptr) {
                        entry->GetNext()->SetPrev(prev);
                    }
                    delete entry;
                    return;
                }
                prev = entry;
                entry = entry->GetNext();
            }
            if (!IsReHashing()) {
                break;
            }
        }
    }

    HashTable::Bucket HashTable::FindBucketByKey(const char *key, HashLenType len, Position pos) noexcept {
        uint64_t hash_key = m_hash_func_(key, len);
        if (IsReHashing()) {
            ReHash();
        }

        if (NeedExpend()) {
            ExpendOrReduce(ExpendSize());
        }

        std::unique_ptr<HashEntryBase *[]> *a[2]{&m_table_, &m_rehash_table_};
        for (int i = 0; i < 2; i++) {
            uint64_t cur_key = hash_key & GetKeyMask(m_size_[i]);

            HashEntryBase *entry = (*a[i]).get()[cur_key];
            while (entry != nullptr) {
                if (entry->GetKey() == key) {
                    if (pos != nullptr) {
                        *pos = entry;
                    }
                    return nullptr;
                }
                entry = entry->GetNext();
            }
            if (!IsReHashing()) {
                break;
            }
        }
        Bucket res;
        res = IsReHashing() ? (m_rehash_table_.get() + (hash_key & GetKeyMask(m_size_[1])))
                            : (m_table_.get() + (hash_key & GetKeyMask(m_size_[0])));

        return res;
    }

    void HashTable::ReHash() noexcept {
        if (!IsReHashing()) {
            return;
        }
        assert(this->m_rehash_table_ != nullptr);
        Bucket bucket = m_table_.get() + m_rehash_idx_;
        HashEntryBase *entry = *bucket;
        while (entry != nullptr) {
            auto next = entry->GetNext();
            Bucket rehash_bucket;
            if (m_size_[0] < m_size_[1]) {
                uint64_t hash_key = m_hash_func_(entry->GetKey().c_str(), entry->GetKey().size());
                rehash_bucket = m_rehash_table_.get() + (hash_key & GetKeyMask(m_size_[1]));
            } else {
                rehash_bucket = m_rehash_table_.get() + (m_rehash_idx_ & GetKeyMask(m_size_[1]));
            }
            entry->SetNext(*rehash_bucket);
            entry->SetPrev(nullptr);
            if (*rehash_bucket != nullptr) {
                (*rehash_bucket)->SetPrev(entry);
            }
            *rehash_bucket = entry;
            entry = next;
            --m_used_[0];
            ++m_used_[1];
        }
        *bucket = nullptr;
        if (m_used_[0] == 0) {
            this->m_rehash_idx_ = -1;
            m_used_[0] = m_used_[1];
            m_size_[0] = m_size_[1];
            m_used_[1] = 0;
            m_size_[1] = 0;
            m_table_ = std::move(m_rehash_table_);
            return;
        }
        ++this->m_rehash_idx_;
    }

    bool HashTable::NeedExpend() const noexcept {
        return 0 == m_size_[0] ||
               (!IsReHashing() && HashLoadFactor <= (m_used_[0] / static_cast<double>((1 << m_size_[0]))));
    }

    bool HashTable::NeedReduce() const noexcept {
        return (!IsReHashing()) && HashLoadFactorLb >= (m_used_[0] / static_cast<double>((1 << m_size_[0])));
    }

    int32_t HashTable::ExpendSize() const noexcept {
        if (0 == m_size_[0]) {
            return 4;
        }
        uint64_t target = m_used_[0] << 2;
        int32_t temp = m_size_[0];
        do {
            ++temp;
        } while ((1 << temp) <= target);
        return temp;
    }

    void HashTable::ExpendOrReduce(int32_t len) noexcept {
        if (0 == m_size_[0]) {
            m_table_ = std::make_unique<HashEntryBase *[]>(1 << len);
            std::fill(m_table_.get(), m_table_.get() + (1 << len), nullptr);
            m_size_[0] = len;
            return;
        }
        m_rehash_table_ = std::make_unique<HashEntryBase *[]>(1 << len);
        std::fill(m_rehash_table_.get(), m_rehash_table_.get() + (1 << len), nullptr);
        m_size_[1] = len;
        m_rehash_idx_ = 0;
    }

    void HashTable::HashIterator::Next() noexcept {
        if (!IsValid()) {
            return;
        }
        m_cur_element_ = m_cur_element_->GetNext();
        while (m_cur_element_ == nullptr) {
            int table_index = (m_table_ == m_hash_table_->m_table_.get()) ? 0 : 1;
            size_t max_size = 1 << m_hash_table_->m_size_[table_index];
            if (m_cur_bucket_ < max_size - 1) {
                ++m_cur_bucket_;
                m_cur_element_ = (m_table_ + m_cur_bucket_)[0];
                assert(m_cur_element_ == nullptr || m_cur_element_->GetPrev() == nullptr);
                continue;
            }

            if (table_index == 0 && m_hash_table_->m_rehash_idx_ != -1) {
                m_table_ = m_hash_table_->m_rehash_table_.get();
                m_cur_bucket_ = 0;
                m_cur_element_ = *m_table_;
                continue;
            }
            break;
        }
    }

    void HashTable::HashIterator::Prev() noexcept {
        if (!IsValid()) {
            return;
        }

        if (m_cur_element_ = m_cur_element_->GetPrev(); m_cur_element_ != nullptr) {
            return;
        }
        do {
            if (m_cur_bucket_ > 0) {
                --m_cur_bucket_;
                m_cur_element_ = *(m_table_ + m_cur_bucket_);
                assert(m_cur_element_ == nullptr || m_cur_element_->GetPrev() == nullptr);
                continue;
            }

            int table_index = (m_table_ == m_hash_table_->m_table_.get()) ? 0 : 1;
            if (table_index == 1) {
                m_table_ = m_hash_table_->m_table_.get();
                m_cur_bucket_ = (1 << m_hash_table_->m_size_[0]) - 1;
                m_cur_element_ = *(m_table_ + m_cur_bucket_);
                continue;
            }
            break;

        } while (m_cur_element_ == nullptr);

        if (m_cur_element_ != nullptr) {
            while (m_cur_element_->GetNext() != nullptr) {
                m_cur_element_ = m_cur_element_->GetNext();
            }
        }

        assert(m_cur_element_ == nullptr || m_cur_element_->GetNext() == nullptr);
    }

    void HashTable::HashIterator::MReset() noexcept {
        if (nullptr == m_hash_table_) {
            return;
        }
        m_cur_bucket_ = 0;
        m_cur_element_ = nullptr;
        size_t max_size = (1 << m_hash_table_->m_size_[0]);
        if (m_hash_table_ != nullptr && 0 < m_hash_table_->Size()) {
            m_table_ = m_hash_table_->m_table_.get();
            m_cur_element_ = m_hash_table_->m_table_.get()[m_cur_bucket_];

            while (m_cur_element_ == nullptr) {
                if (m_cur_bucket_ < max_size - 1) {
                    ++m_cur_bucket_;
                    m_cur_element_ = m_hash_table_->m_table_.get()[m_cur_bucket_];
                    continue;
                }

                if (m_hash_table_->m_rehash_idx_ != -1) {
                    m_table_ = m_hash_table_->m_rehash_table_.get();
                    m_cur_bucket_ = 0;
                    m_cur_element_ = m_hash_table_->m_table_.get()[m_cur_bucket_];

                    max_size = (1 << m_hash_table_->m_size_[1]);
                    continue;
                }
                break;
            }
            return;
        }
        m_table_ = nullptr;
    }
}  // namespace clsn
