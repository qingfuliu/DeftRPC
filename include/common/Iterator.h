//
// Created by lqf on 23-6-20.
//

#ifndef DEFTRPC_ITERATOR_H
#define DEFTRPC_ITERATOR_H

namespace CLSN {

    class Iterator {
    public:
        Iterator() = default;

        virtual ~Iterator() = default;

        virtual bool IsValid() const noexcept = 0;

        virtual void Next() noexcept = 0;

        virtual void Prev() noexcept = 0;

        virtual void *Get() const noexcept = 0;

        void *operator()() const noexcept {
            return Get();
        }
    };
}

#endif //DEFTRPC_ITERATOR_H
