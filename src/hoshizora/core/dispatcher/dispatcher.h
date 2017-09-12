#ifndef HOSHIZORA_DISPATCHER_H
#define HOSHIZORA_DISPATCHER_H

#include "hoshizora/core/util/includes.h"

namespace hoshizora {
    template<class Kernel>
    class Dispatcher {
        //template<class Result>
        virtual string run() = 0;
    };
}


#endif //HOSHIZORA_DISPATCHER_H
