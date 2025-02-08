//
// Created by lqf on 23-4-18.
//

#ifndef DEFTRPC_TASK_H
#define DEFTRPC_TASK_H

#include <functional>

namespace clsn {
using Task = std::function<void(void)>;
}  // namespace clsn

#endif  // DEFTRPC_TASK_H
