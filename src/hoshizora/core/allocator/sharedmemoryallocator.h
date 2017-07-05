#ifndef HOSHIZORA_SHAREDMEMORYEXECUTOR_H
#define HOSHIZORA_SHAREDMEMORYEXECUTOR_H

#include "hoshizora/core/allocator/allocator.h"

namespace hoshizora {
    struct SharedMemoryAllocator : Allocator {
        static void *alloc() {
            std::cout << "Shared memory allocator" << std::endl;
            return nullptr;
        }
    };
}


#endif //HOSHIZORA_SHAREDMEMORYEXECUTOR_H
