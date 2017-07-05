#ifndef HOSHIZORA_EXECUTOR_H
#define HOSHIZORA_EXECUTOR_H

#include "hoshizora/core/util/includes.h"

namespace hoshizora {
    struct Allocator {
        static void *alloc() {
            std::cout << "Executor" << std::endl;
            return nullptr;
        };
    };
}


#endif //HOSHIZORA_EXECUTOR_H
