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

    class RpcExecuteException : public std::logic_error {
    public:
        explicit RpcExecuteException(const std::string &str) :
                std::logic_error(str) {}

//        explicit RpcExecuteException(std::string &&str) :
//                std::logic_error(std::move(str)) {}

        RpcExecuteException(const RpcExecuteException &) noexcept = default;

        ~RpcExecuteException() override = default;

        RpcExecuteException &operator=(const RpcExecuteException &) noexcept = default;
    };

    inline void HandleException(std::string &res, std::exception_ptr eptr) {
        try {
            if (eptr != nullptr) {
                std::rethrow_exception(eptr);
            }
        } catch (std::exception &e) {
            res.append(e.what());
        }
    }
}

#endif //DEFTRPC_EXCEPTION_H
