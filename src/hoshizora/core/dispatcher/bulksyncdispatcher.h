#ifndef HOSHIZORA_BULKSYNCDISPATCHER_H
#define HOSHIZORA_BULKSYNCDISPATCHER_H

#include <string>
#include "hoshizora/core/dispatcher/dispatcher.h"

namespace hoshizora {
    template<class Kernel, class Allocator>
    struct BulkSyncDispatcher : Dispatcher<Kernel, Allocator> {
        using Graph = typename Kernel::_Graph;
        using ID = typename Kernel::_Graph::_ID;

        Graph prev_graph;
        Graph curr_graph;

        BulkSyncDispatcher(Graph &graph) {
//            prev_graph = graph;
//            curr_graph = Graph::Next(graph);

            prev_graph = graph;
            curr_graph = graph;
            curr_graph.set_v_data();
            curr_graph.set_e_data();

//            debug::print(prev_graph.v_data);
//            debug::print(curr_graph.v_data);
        }

        std::string run() {
            Kernel kernel;
            const auto num_vertices = prev_graph.num_vertices;
            const auto num_edges = prev_graph.num_edges;

            constexpr auto num_iters = 1u;
            for (auto iter = 0u; iter < num_iters; ++iter) {
                if (iter == 0) {
                    // init
                    for (ID src = 0; src < num_vertices; ++src) {
                        for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                            const auto index = prev_graph.out_offsets[src] + i;
                            prev_graph.v_data[index] = kernel.init(src, prev_graph);
                        }
                    }
                } else {
                    Graph::Next(prev_graph, curr_graph);
                }

                // scatter
                for (ID src = 0; src < num_vertices; ++src) {
                    for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                        const auto dst = prev_graph.out_neighbors[src][i];
                        const auto index = prev_graph.out_offsets[src] + i;
                        curr_graph.e_data[prev_graph.forward_indices[index]]
                                = kernel.scatter(src, dst, prev_graph.v_data[src], prev_graph);
                    }
                }

                // gather
                for (ID src = 0; src < num_vertices; ++src) {
                    for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                        const auto dst = prev_graph.out_neighbors[src][i];
                        const auto index = prev_graph.out_offsets[src] + i;
                        curr_graph.e_data[index]
                                = kernel.gather(src, dst,
                                                prev_graph.e_data[index],
                                                curr_graph.e_data[index],
                                                prev_graph);
                    }
                }

                // sum and apply
                for (ID dst = 0; dst < num_vertices; ++dst) {
                    curr_graph.v_data[dst] = kernel.zero(dst, prev_graph); // TODO
                    for (ID i = 0, end = prev_graph.in_degrees[dst]; i < end; ++i) {
                        const auto src = prev_graph.in_neighbors[dst][i];
                        const auto index = prev_graph.in_offsets[dst] + i;
                        curr_graph.v_data[dst] = kernel.sum(dst, src,
                                                            curr_graph.v_data[dst],
                                                            curr_graph.e_data[index], prev_graph);
                    }
                    curr_graph.v_data[dst] = kernel.apply(dst,
                                                          prev_graph.v_data[dst],
                                                          curr_graph.v_data[dst], prev_graph);
                }
            }

            return kernel.result(curr_graph);
        }
    };
}


#endif //HOSHIZORA_BULKSYNCDISPATCHER_H
