//
// Created by lqf on 23-5-14.
//

#ifndef DEFTRPC_HASH_H
#define DEFTRPC_HASH_H

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include "common/Iterator.h"
#include "highwayhash/sip_hash.h"

namespace clsn {
    class HashEntryBase {
    public:
        HashEntryBase() noexcept = default;

        virtual ~HashEntryBase() = default;

        [[nodiscard]] const std::string &GetKey() const noexcept { return m_key_; }

        void SetKey(std::string k) noexcept { m_key_ = std::move(k); }

        virtual void *GetVal() noexcept = 0;

        virtual void SetVal(void *) noexcept = 0;

        HashEntryBase *GetNext() noexcept { return m_next_; }

        void SetNext(HashEntryBase *n) noexcept { m_next_ = n; }

        HashEntryBase *GetPrev() noexcept { return m_prev_; }

        void SetPrev(HashEntryBase *n) noexcept { m_prev_ = n; }

    private:
        std::string m_key_;
        HashEntryBase *m_next_{nullptr};
        HashEntryBase *m_prev_{nullptr};
    };

    template<typename T>
    class HashEntry : public HashEntryBase {
        using storage = typename std::aligned_storage_t<sizeof(T), std::alignment_of_v<T>>;

    public:
        HashEntry() noexcept = default;

        explicit HashEntry(T &&v) noexcept: HashEntryBase() {
            if constexpr (std::is_trivial_v<T>) {
                std::copy(reinterpret_cast<char *>(&v), reinterpret_cast<char *>(&v) + sizeof(T),
                          reinterpret_cast<char *>(&m_val_));
            } else {
                new(&m_val_) T(std::move(v));
            }
        }

        explicit HashEntry(T &v) noexcept: HashEntryBase() {
            if constexpr (std::is_trivial_v<T>) {
                std::copy(reinterpret_cast<char *>(&v), reinterpret_cast<char *>(&v) + sizeof(T),
                          reinterpret_cast<char *>(&m_val_));
            } else {
                new(&m_val_) T(v);
            }
        }

        ~HashEntry() override = default;

        void *GetVal() noexcept override { return static_cast<void *>(&m_val_); }

        void SetVal(void *v) noexcept override {
            if constexpr (std::is_trivial_v<T>) {
                std::copy(reinterpret_cast<char *>(v), reinterpret_cast<char *>(v) + sizeof(T),
                          reinterpret_cast<char *>(&m_val_));
            } else {
                new(&m_val_) T(*static_cast<T *>(v));
            }
        }

    private:
        storage m_val_{};
    };

    class HashTable {
        class HashIterator;

        using HashLenType = std::uint64_t;
        static inline constexpr highwayhash::SipHashState::Key M_SIP_HASH_KEY = {0x0706050403020100ULL,
                                                                                 0x0F0E0D0C0B0A0908ULL};
        using Bucket = HashEntryBase **;
        using Position = HashEntryBase **;
        using EntryType = HashEntryBase *;

    public:
        ~HashTable() noexcept {
            {
                auto f = [](EntryType entry) -> void { delete entry; };
                ForEachTable(m_table_.get(), 1 << m_size_[0], std::function<void(EntryType)>(std::move(f)));
            }

            if (IsReHashing()) {
                {
                    auto f = [](EntryType entry) -> void { delete entry; };
                    ForEachTable(m_rehash_table_.get(), 1 << m_size_[1], std::move(f));
                }
            }
        }

        template<class T>
        EntryType Insert(const std::string &str, T &&value) noexcept {
            return Insert(str.c_str(), str.size(), std::forward<T>(value));
        }

        template<class T>
        EntryType Insert(const char *key, HashLenType len, T &&value) noexcept {
            auto bucket = FindBucketByKey(key, len, nullptr);
            if (bucket == nullptr) {
                return nullptr;
            }
            return InsertToBucket(key, std::forward<T>(value), bucket);
        }

        HashEntryBase *FindByKey(const std::string &str) noexcept { return FindByKey(str.c_str(), str.size()); }

        /**
         *
         * @param key
         * @param len
         * @return nullptr or HashEntryBase*
         */
        HashEntryBase *FindByKey(const char *key, HashLenType len) noexcept;

        void Delete(const std::string &str) noexcept { Delete(str.c_str(), str.size()); }

        void Delete(const char *key, HashLenType len) noexcept;

        [[nodiscard]] bool IsEmpty() const noexcept { return (m_used_[0] + m_used_[1]) == 0; }

        [[nodiscard]] int32_t Size() const noexcept { return m_used_[0] + m_used_[1]; }

        std::unique_ptr<clsn::Iterator> GetIterator() noexcept { return std::make_unique<HashIterator>(this); }

        static const std::string &GetKeyByIterator(Iterator *it) {
            if (!it->IsValid()) {
                throw std::logic_error("hash entry is invalid.");
            }
            return static_cast<HashEntryBase *>(it->Get())->GetKey();
        }

        static void *GetValByIterator(Iterator *it) noexcept {
            if (it->IsValid()) {
                return static_cast<HashEntryBase *>(it->Get())->GetVal();
            }
            return nullptr;
        }

    private:
        template<class T>
        EntryType InsertToBucket(const char *key, T &&value, Bucket bucket) noexcept {
            using EntryT = std::decay_t<T>;
            int i = IsReHashing() ? 1 : 0;
            ++m_used_[i];
            auto entry = new HashEntry<EntryT>(std::forward<T>(value));
            entry->SetKey(key);
            entry->SetNext(*bucket);
            entry->SetPrev(nullptr);
            if (*bucket != nullptr) {
                (*bucket)->SetPrev(entry);
            }
            *bucket = entry;
            return entry;
        }

        Bucket FindBucketByKey(const char *key, HashLenType len, Position pos) noexcept;

        void ReHash() noexcept;

        [[nodiscard]] int32_t ExpendSize() const noexcept;

        [[nodiscard]] bool NeedExpend() const noexcept;

        [[nodiscard]] bool NeedReduce() const noexcept;

        void ExpendOrReduce(int32_t len) noexcept;

        [[nodiscard]] static uint64_t GetKeyMask(uint64_t len) noexcept { return (1 << len) - 1; }

        [[nodiscard]] bool IsReHashing() const noexcept { return -1 != m_rehash_idx_; }

        static void ForEachTable(Bucket bucket, int tableSize, const std::function<void(EntryType)> &func) {
            for (auto i = 0; i < tableSize; ++i) {
                ForEachBucket(bucket + i, func);
            }
        }

        static void ForEachBucket(Bucket bucket, const std::function<void(EntryType)> &func) noexcept {
            HashEntryBase *entry = *bucket;
            while (entry != nullptr) {
                auto next = entry->GetNext();
                func(entry);
                entry = next;
            };
        }

    private:
        class HashIterator : public Iterator {
        public:
            explicit HashIterator(HashTable *hash) noexcept: m_hash_table_(hash) {
                if (m_hash_table_ != nullptr && !m_hash_table_->IsEmpty()) {
                    MReset();
                }
            }

            ~HashIterator() override = default;

            HashIterator(const HashIterator &) noexcept = default;

            HashIterator &operator()(const HashIterator &other) noexcept {
                Iterator::operator=(other);
                m_table_ = other.m_table_;
                m_cur_element_ = other.m_cur_element_;
                m_cur_bucket_ = other.m_cur_bucket_;
                m_hash_table_ = other.m_hash_table_;
                return *this;
            }

            [[nodiscard]] bool IsValid() const noexcept override { return m_cur_element_ != nullptr; }

            void Next() noexcept override;

            void Prev() noexcept override;

            [[nodiscard]] void *Get() const noexcept override {
                if (!IsValid()) {
                    return nullptr;
                }
                return m_cur_element_;
            }

            void Reset() noexcept override { MReset(); }

        private:
            void MReset() noexcept;

        private:
            HashEntryBase **m_table_{nullptr};
            size_t m_cur_bucket_{0};
            HashEntryBase *m_cur_element_{nullptr};
            HashTable *m_hash_table_{nullptr};
        };

    private:
        std::unique_ptr<HashEntryBase *[]> m_table_{nullptr};
        std::unique_ptr<HashEntryBase *[]> m_rehash_table_{nullptr};
        int32_t m_size_[2]{0, 0};
        int32_t m_used_[2]{0, 0};
        int32_t m_rehash_idx_{-1};
        std::function<HashLenType(const char *, HashLenType)> m_hash_func_ = [](const char *data,
                                                                                HashLenType len) -> HashLenType {
            return highwayhash::SipHash(HashTable::M_SIP_HASH_KEY, data, len);
        };
    };
}  // namespace clsn

#endif  // DEFTRPC_HASH_H
