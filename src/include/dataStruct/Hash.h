//
// Created by lqf on 23-5-14.
//

#ifndef DEFTRPC_HASH_H
#define DEFTRPC_HASH_H

#include <cassert>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>
#include "common/Iterator.h"
#include "hash/google_highwayhash_Public/sip_hash.h"

namespace clsn {
class HashEntryBase {
 public:
  HashEntryBase() noexcept = default;

  virtual ~HashEntryBase() = default;

  [[nodiscard]] const std::string &GetKey() const noexcept { return key; }

  void SetKey(std::string k) noexcept { key = std::move(k); }

  virtual void *GetVal() noexcept = 0;

  virtual void SetVal(void *) noexcept = 0;

  HashEntryBase *GetNext() noexcept { return next; }

  void SetNext(HashEntryBase *n) noexcept { next = n; }

  HashEntryBase *GetPrev() noexcept { return prev; }

  void SetPrev(HashEntryBase *n) noexcept { prev = n; }

 private:
  std::string key;
  HashEntryBase *next{nullptr};
  HashEntryBase *prev{nullptr};
};

template <typename T>
class HashEntry : public HashEntryBase {
  using storage = typename std::aligned_storage<sizeof(T), std::alignment_of_v<T>>::type;

 public:
  HashEntry() noexcept = default;

  explicit HashEntry(T &&v) noexcept : HashEntryBase() {
    if constexpr (std::is_trivial_v<T>) {
      std::copy(reinterpret_cast<char *>(&v), reinterpret_cast<char *>(&v) + sizeof(T), reinterpret_cast<char *>(&val));
    } else {
      new (&val) T(std::move(v));
    }
  }

  explicit HashEntry(T &v) noexcept : HashEntryBase() {
    if constexpr (std::is_trivial_v<T>) {
      std::copy(reinterpret_cast<char *>(&v), reinterpret_cast<char *>(&v) + sizeof(T), reinterpret_cast<char *>(&val));
    } else {
      new (&val) T(v);
    }
  }

  ~HashEntry() override = default;

  void *GetVal() noexcept override { return static_cast<void *>(&val); }

  void SetVal(void *v) noexcept override {
    if constexpr (std::is_trivial_v<T>) {
      std::copy(reinterpret_cast<char *>(v), reinterpret_cast<char *>(v) + sizeof(T), reinterpret_cast<char *>(&val));
    } else {
      new (&val) T(*static_cast<T *>(v));
    }
  }

 private:
  storage val{};
};

class HashTable {
  class HashIterator;

  using HashLenType = unsigned long long;
  const HashLenType sipHashKey[2] = {0x0706050403020100ULL, 0x0F0E0D0C0B0A0908ULL};
  using Bucket = HashEntryBase **;
  using Position = HashEntryBase **;
  using EntryType = HashEntryBase *;

 public:
  ~HashTable() noexcept {
    {
      auto f = [](EntryType entry) -> void { delete entry; };
      forEachTable(table.get(), 1 << size[0], std::function<void(EntryType)>(std::move(f)));
    }

    if (isReHashing()) {
      {
        auto f = [](EntryType entry) -> void { delete entry; };
        forEachTable(reHashTable.get(), 1 << size[1], std::move(f));
      }
    }
  }

  template <class T>
  EntryType Insert(const std::string &str, T &&value) noexcept {
    return Insert(str.c_str(), str.size(), std::forward<T>(value));
  }

  template <class T>
  EntryType Insert(const char *key, HashLenType len, T &&value) noexcept {
    auto bucket = FindBucketByKey(key, len, nullptr);
    if (bucket == nullptr) {
      return nullptr;
    }
    return insertToBucket(key, std::forward<T>(value), bucket);
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

  [[nodiscard]] bool IsEmpty() const noexcept { return (used[0] + used[1]) == 0; }

  [[nodiscard]] int32_t Size() const noexcept { return used[0] + used[1]; }

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
  template <class T>
  EntryType insertToBucket(const char *key, T &&value, Bucket bucket) noexcept {
    using EntryT = std::decay_t<T>;
    int i = isReHashing() ? 1 : 0;
    ++used[i];
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

  void reHash() noexcept;

  [[nodiscard]] int32_t ExpendSize() const noexcept;

  [[nodiscard]] bool needExpend() const noexcept;

  [[nodiscard]] bool needReduce() const noexcept;

  void ExpendOrReduce(int32_t len) noexcept;

  [[nodiscard]] static uint64_t getKeyMask(uint64_t len) noexcept { return (1 << len) - 1; }

  [[nodiscard]] bool isReHashing() const noexcept { return -1 != reHashIdx; }

  static void forEachTable(Bucket bucket, int tableSize, const std::function<void(EntryType)> &func) {
    for (auto i = 0; i < tableSize; ++i) {
      forEachBucket(bucket + i, func);
    }
  }

  static void forEachBucket(Bucket bucket, const std::function<void(EntryType)> &func) noexcept {
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
    explicit HashIterator(HashTable *hash) noexcept
        : Iterator(), table(nullptr), curBucket(0), curElement(nullptr), hashTable(hash) {
      if (hashTable != nullptr && !hashTable->IsEmpty()) {
        reset();
      }
    }

    ~HashIterator() override = default;

    HashIterator(const HashIterator &) noexcept = default;

    HashIterator &operator()(const HashIterator &other) noexcept {
      Iterator::operator=(other);
      table = other.table;
      curElement = other.curElement;
      curBucket = other.curBucket;
      hashTable = other.hashTable;
      return *this;
    }

    [[nodiscard]] bool IsValid() const noexcept override { return curElement != nullptr; }

    void Next() noexcept override;

    void Prev() noexcept override;

    [[nodiscard]] void *Get() const noexcept override {
      if (!IsValid()) {
        return nullptr;
      }
      return curElement;
    }

    void Reset() noexcept override { reset(); }

   private:
    void reset() noexcept;

   private:
    HashEntryBase **table;
    size_t curBucket;
    HashEntryBase *curElement;
    HashTable *hashTable;
  };

 private:
  std::unique_ptr<HashEntryBase *[]> table{nullptr};
  std::unique_ptr<HashEntryBase *[]> reHashTable{nullptr};
  int32_t size[2]{0, 0};
  int32_t used[2]{0, 0};
  int32_t reHashIdx{-1};
  std::function<HashLenType(const char *, HashLenType)> hashFunc =
      [this](const char *data, HashLenType len) -> HashLenType { return highwayhash::SipHash(sipHashKey, data, len); };
};
}  // namespace clsn

#endif  // DEFTRPC_HASH_H
