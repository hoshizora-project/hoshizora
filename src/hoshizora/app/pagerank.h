#ifndef HOSHIZORA_PAGERANK_H
#define HOSHIZORA_PAGERANK_H


#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/api/kernel.h"

namespace hoshizora {
    template<class Graph>
    struct PageRankKernel : Kernel<Graph> {
        using _Graph = Graph;
        using EData = typename Graph::_EData;
        using VData = typename Graph::_VData;
        using ID = typename Graph::_ID;

        constexpr static auto RANDOM_RESET_PROP = 0.15;

        VData init(const ID src, const ID dst, const Graph &graph) {
            return 1.0;
        }

        EData scatter(const ID src, const ID dst,
                      const VData v_val, const Graph &graph) {
//            std::cout << src << "->" << dst << ": " << v_val << ", " << graph.out_degrees[src]
//                      << std::endl;
            return v_val / graph.out_degrees[src];
        }

        EData gather(const ID src, const ID dst, const VData prev_val,
                     const VData curr_val, const Graph &graph) {
//            std::cout << src << "->" << dst << ": " << curr_val << std::endl;
            return curr_val;
        }

        VData sum(const ID src, const ID dst,
                  const VData v_val, const EData e_val, const Graph &graph) {
//            std::cout << src << "->" << dst << ": " << v_val << ", " << e_val << std::endl;
            return v_val + e_val;
        }

        VData apply(const ID dst, const VData prev_val,
                    const VData curr_val, const Graph &graph) {
//            return RANDOM_RESET_PROP / graph.num_vertices
//                   + (1 - RANDOM_RESET_PROP) * (prev_val + curr_val);
            std::cout << "->" << dst << ": " << prev_val << ", " << curr_val << std::endl;
            return prev_val + curr_val;
        }

        string result(const Graph &graph) {
            string res = "";
            for (ID src = 0; src < graph.num_vertices; ++src) {
                res = res + to_string(src) + ": " + to_string(graph.v_data[src]) + "\n";
            }
            return res;
        }
    };
}


#endif //HOSHIZORA_PAGERANK_H
