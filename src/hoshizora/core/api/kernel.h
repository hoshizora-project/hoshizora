#ifndef HOSHIZORA_KERNEL_H
#define HOSHIZORA_KERNEL_H

#include "hoshizora/core/util/includes.h"

namespace hoshizora {
    template<class Graph>
    struct Kernel {
        using EData = typename Graph::_EData;
        using VData = typename Graph::_VData;
        using ID = typename Graph::_ID;

        virtual VData init() = 0;
        virtual EData scatter(ID src, ID dst, VData val) = 0;
        virtual EData gather(ID src, ID dst, VData v_val, EData e_val) = 0;
        virtual VData sum(ID src, ID dst, VData v_val, EData e_val) = 0;
        virtual VData apply(ID dst, VData prev_val, VData curr_val) = 0;
    };
}

#endif //HOSHIZORA_KERNEL_H
