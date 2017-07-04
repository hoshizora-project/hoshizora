#ifndef HOSHIZORA_PAGERANK_H
#define HOSHIZORA_PAGERANK_H


#include <hoshizora/core/util/includes.h>
#include <hoshizora/core/api/kernel.h>

namespace hoshizora {
    template<class Graph>
    struct PageRankKernel : Kernel<Graph> {
        using _Graph = Graph;
        using EData = typename Graph::_EData;
        using VData = typename Graph::_VData;
        using ID = typename Graph::_ID;


        VData init() {
            return 0.0;
        }

        EData scatter(ID src, ID dst, VData v_val) {
            return v_val;
        }

        EData gather(ID src, ID dst, VData v_val, EData e_val) {
            return v_val;
        }

        VData sum(ID src, ID dst, VData v_val, EData e_val) {
            return v_val;
        }

        VData apply(ID dst, VData prev_val, VData curr_val) {
            return prev_val;
        }
    };
}


#endif //HOSHIZORA_PAGERANK_H
