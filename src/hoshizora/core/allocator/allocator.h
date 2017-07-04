#ifndef HOSHIZORA_EXECUTOR_H
#define HOSHIZORA_EXECUTOR_H


namespace hoshizora {
    struct Allocator {
        static void *alloc() {
            std::cout << "Executor" << std::endl;
            return nullptr;
        };
    };
}


#endif //HOSHIZORA_EXECUTOR_H
