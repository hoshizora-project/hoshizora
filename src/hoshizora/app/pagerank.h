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

        constexpr static auto JUMP_PROB = 0.15;

        VData init(const ID src, const Graph &graph) {
//            return 1.0 / graph.num_vertices;
            return 1.0;
        }

        EData scatter(const ID src, const ID dst,
                      const VData v_val, const Graph &graph) {
            return v_val / graph.out_degrees(src, 0); // TODO: numa_id
        }

        EData gather(const ID src, const ID dst, const VData prev_val,
                     const VData curr_val, const Graph &graph) {
            return curr_val;
        }

        VData zero(const ID dst, const Graph &graph) {
            return 0.0;
        }

        VData sum(const ID src, const ID dst,
                  const VData v_val, const EData e_val, const Graph &graph) {
            return v_val + e_val;
        }

        VData apply(const ID dst, const VData prev_val,
                    const VData curr_val, const Graph &graph) {
            return (1 - JUMP_PROB) * curr_val + JUMP_PROB / graph.num_vertices;
        }

        string result(const Graph &graph) {
//            debug::print(graph.num_vertices);

/*
            string res = "{\"nodes\":[";
            for (ID i = 0; i < graph.num_vertices; i += 1000) {
                res = res +
                        "{\"id\":" +to_string(i) + ", \"pagerank\":" + to_string(graph.v_data[i]) + "},\n";
            }
            res = res + "]}";

            */
            string res = "";

            return res;
        }
    };
}


#endif //HOSHIZORA_PAGERANK_H
