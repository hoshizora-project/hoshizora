#ifndef HOSHIZORA_DISPATCHER_H
#define HOSHIZORA_DISPATCHER_H


namespace hoshizora {
    template<class Kernel, class Executor>
    class Dispatcher {
        //template<class Result>
        virtual string run() = 0;
    };
}


#endif //HOSHIZORA_DISPATCHER_H
