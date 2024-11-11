//
// Created by lqf on 23-5-14.
//
#include "dataStruct/Hash.h"
#include <cassert>
#include <utility>
#include "dataStructConfig.h"

namespace CLSN {

HashEntryBase *HashTable::FindByKey(const char *key, HashLenType len) noexcept {
  HashEntryBase *p;
  auto bucket = FindBucketByKey(key, len, &p);
  if (bucket == nullptr) {
    return p;
  }
  return nullptr;
}

void HashTable::Delete(const char *key, unsigned long long len) noexcept {
  uint64_t mKey = hashFunc(key, len);
  if (isReHashing()) {
    reHash();
  }

  if (needReduce()) {
    ExpendOrReduce(ExpendSize());
  }

  std::unique_ptr<HashEntryBase *[]> *a[2]{&table, &reHashTable};
  for (int i = 0; i < 2; i++) {
    uint64_t curKey = mKey & getKeyMask(size[i]);

    HashEntryBase *entry = (*a[i]).get()[curKey];
    HashEntryBase *prev = nullptr;
    while (entry != nullptr) {
      if (entry->GetKey() == key) {
        --used[i];
        if (prev == nullptr) {
          (*a[i]).get()[curKey] = entry->GetNext();
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
    if (!isReHashing()) {
      break;
    }
  }
}

HashTable::Bucket HashTable::FindBucketByKey(const char *key, HashLenType len, Position pos) noexcept {
  uint64_t mKey = hashFunc(key, len);
  if (isReHashing()) {
    reHash();
  }

  if (needExpend()) {
    ExpendOrReduce(ExpendSize());
  }

  std::unique_ptr<HashEntryBase *[]> *a[2]{&table, &reHashTable};
  for (int i = 0; i < 2; i++) {
    uint64_t curKey = mKey & getKeyMask(size[i]);

    HashEntryBase *entry = (*a[i]).get()[curKey];
    while (entry != nullptr) {
      if (entry->GetKey() == key) {
        if (pos != nullptr) {
          *pos = entry;
        }
        return nullptr;
      }
      entry = entry->GetNext();
    }
    if (!isReHashing()) {
      break;
    }
  }
  Bucket res;
  res =
      isReHashing() ? (reHashTable.get() + (mKey & getKeyMask(size[1]))) : (table.get() + (mKey & getKeyMask(size[0])));

  return res;
}

void HashTable::reHash() noexcept {
  if (!isReHashing()) {
    return;
  }
  assert(this->reHashTable != nullptr);
  Bucket bucket = table.get() + reHashIdx;
  HashEntryBase *entry = *bucket;
  while (entry != nullptr) {
    auto next = entry->GetNext();
    Bucket rehashBucket;
    if (size[0] < size[1]) {
      uint64_t mKey = hashFunc(entry->GetKey().c_str(), entry->GetKey().size());
      rehashBucket = reHashTable.get() + (mKey & getKeyMask(size[1]));
    } else {
      rehashBucket = reHashTable.get() + (reHashIdx & getKeyMask(size[1]));
    }
    entry->SetNext(*rehashBucket);
    entry->SetPrev(nullptr);
    if (*rehashBucket != nullptr) {
      (*rehashBucket)->SetPrev(entry);
    }
    *rehashBucket = entry;
    entry = next;
    --used[0];
    ++used[1];
  }
  *bucket = nullptr;
  if (used[0] == 0) {
    this->reHashIdx = -1;
    used[0] = used[1];
    size[0] = size[1];
    used[1] = 0;
    size[1] = 0;
    table = std::move(reHashTable);
    return;
  }
  ++this->reHashIdx;
}

bool HashTable::needExpend() const noexcept {
  return 0 == size[0] || (!isReHashing() && HashLoadFactor <= (used[0] / static_cast<double>((1 << size[0]))));
}

bool HashTable::needReduce() const noexcept {
  return (!isReHashing()) && HashLoadFactorLb >= (used[0] / static_cast<double>((1 << size[0])));
}

int32_t HashTable::ExpendSize() const noexcept {
  if (0 == size[0]) {
    return 4;
  }
  uint64_t target = used[0] << 2;
  int32_t temp = size[0];
  do {
    ++temp;
  } while ((1 << temp) <= target);
  return temp;
}

void HashTable::ExpendOrReduce(int32_t len) noexcept {
  if (0 == size[0]) {
    table = std::make_unique<HashEntryBase *[]>(1 << len);
    std::fill(table.get(), table.get() + (1 << len), nullptr);
    size[0] = len;
    return;
  }
  reHashTable = std::make_unique<HashEntryBase *[]>(1 << len);
  std::fill(reHashTable.get(), reHashTable.get() + (1 << len), nullptr);
  size[1] = len;
  reHashIdx = 0;
}

void HashTable::HashIterator::Next() noexcept {
  if (!IsValid()) {
    return;
  }
  curElement = curElement->GetNext();
  while (curElement == nullptr) {
    int tableIndex = (table == hashTable->table.get()) ? 0 : 1;
    size_t maxSize = 1 << hashTable->size[tableIndex];
    if (curBucket < maxSize - 1) {
      ++curBucket;
      curElement = (table + curBucket)[0];
      assert(curElement == nullptr || curElement->GetPrev() == nullptr);
      continue;
    }

    if (tableIndex == 0 && hashTable->reHashIdx != -1) {
      table = hashTable->reHashTable.get();
      curBucket = 0;
      curElement = *table;
      continue;
    }
    break;
  }
}

void HashTable::HashIterator::Prev() noexcept {
  if (!IsValid()) {
    return;
  }

  if (curElement = curElement->GetPrev(); curElement != nullptr) {
    return;
  }
  do {
    if (curBucket > 0) {
      --curBucket;
      curElement = *(table + curBucket);
      assert(curElement == nullptr || curElement->GetPrev() == nullptr);
      continue;
    }

    int tableIndex = (table == hashTable->table.get()) ? 0 : 1;
    if (tableIndex == 1) {
      table = hashTable->table.get();
      curBucket = (1 << hashTable->size[0]) - 1;
      curElement = *(table + curBucket);
      continue;
    }
    break;

  } while (curElement == nullptr);

  if (curElement != nullptr) {
    while (curElement->GetNext() != nullptr) {
      curElement = curElement->GetNext();
    }
  }

  assert(curElement == nullptr || curElement->GetNext() == nullptr);
}

void HashTable::HashIterator::reset() noexcept {
  curBucket = 0;
  curElement = nullptr;

  size_t maxSize = (1 << hashTable->size[0]);
  if (hashTable != nullptr && 0 < hashTable->Size()) {
    table = hashTable->table.get();
    curElement = hashTable->table.get()[curBucket];

    while (curElement == nullptr) {
      if (curBucket < maxSize - 1) {
        ++curBucket;
        curElement = hashTable->table.get()[curBucket];
        continue;
      }

      if (hashTable->reHashIdx != -1) {
        table = hashTable->reHashTable.get();
        curBucket = 0;
        curElement = hashTable->table.get()[curBucket];

        maxSize = (1 << hashTable->size[1]);
        continue;
      }
      break;
    }
    return;
  }
  table = nullptr;
}
}  // namespace CLSN
