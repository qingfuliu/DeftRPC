//
// Created by lqf on 23-5-14.
//
#include"dataStruct/hash.h"
#include "log/Log.h"

using namespace CLSN;

struct TestEntry {
    int a;
    int b;
};

int main() {
    CLSN::init<0>({
                          CLSN::createConsoleLogAppender(
                                  "[%t] %Y-%m-%d %H:%M:%S:<%f:%n> [%l] %s",
                                  CLSN::LogLevel::Debug)});
    HashTable t;
    std::vector<std::string> vec(5000);
    CLSN_LOG_DEBUG << vec.capacity() << " " << vec.size();
    for (int i = 0; i < vec.size(); i++) {

            vec[i] = "lqf" + std::to_string(i);
            auto res = t.Insert(vec[i], i);
            assert(i == *static_cast<int *>(res->GetVal()));
            assert(t.Size() == i + 1);

            res = t.FindByKey(vec[i]);
            assert(i == *static_cast<int *>(res->GetVal()));

    }
    CLSN_LOG_DEBUG << vec.capacity() << " " << vec.size();
    for (int i = 0; i < vec.size(); i++) {
        assert(nullptr == t.Insert(vec[i], i));
        auto res = t.FindByKey(vec[i]);
        assert(i == *static_cast<int *>(res->GetVal()));
    }

    for (int i = 0; i < vec.size()/2; i++) {
            t.Delete(vec[i]);
            assert(t.Size() == (vec.size() - i - 1));
            auto res = t.FindByKey(vec[i]);
            assert(res == nullptr);
    }
}