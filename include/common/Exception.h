//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_EXCEPTION_H
#define DEFTRPC_EXCEPTION_H

#include<stdexcept>

namespace CLSN {

    class SchedulingException : public std::logic_error {
    public:
        explicit SchedulingException(const std::string &str) :
                std::logic_error(str) {}

        SchedulingException(const SchedulingException &) noexcept = default;

        ~SchedulingException() override = default;

        SchedulingException &operator=(const SchedulingException &) noexcept = default;
    };

}

#endif //DEFTRPC_EXCEPTION_H
