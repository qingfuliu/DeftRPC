//
// Created by lqf on 23-5-14.
//
#include "dataStruct/Hash.h"
#include "dataStructConfig.h"
#include <utility>
#include <cassert>

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

    HashTable::Bucket HashTable::FindBucketByKey(const char *key, HashLenType len,
                                                 Position pos) noexcept {
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
        res = isReHashing() ? (reHashTable.get() + (mKey & getKeyMask(size[1]))) :
              (table.get() + (mKey & getKeyMask(size[0])));

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
                uint64_t mKey = hashFunc(entry->GetKey().c_str(),
                                         entry->GetKey().size());
                rehashBucket = reHashTable.get() +
                               (mKey & getKeyMask(size[1]));
            } else {
                rehashBucket = reHashTable.get() +
                               (reHashIdx & getKeyMask(size[1]));
            }
            entry->SetNext(*rehashBucket);
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
        return 0 == size[0] || (!isReHashing()
                                && HashLoadFactor <= (used[0] / static_cast<double >((1 << size[0]))));
    }

    bool HashTable::needReduce() const noexcept {
        return (!isReHashing()) && HashLoadFactorLb >= (used[0] / static_cast<double >((1 << size[0])));
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
}