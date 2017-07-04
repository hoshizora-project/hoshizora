#ifndef HOSHIZORA_BULKSYNCDISPATCHER_H
#define HOSHIZORA_BULKSYNCDISPATCHER_H

#include<hoshizora/core/dispatcher/dispatcher.h>

namespace hoshizora {
    template<class Kernel, class Allocator>
    struct BulkSyncDispatcher : Dispatcher<Kernel, Allocator> {
        using Graph = typename Kernel::_Graph;
        using ID = typename Kernel::_Graph::_ID;

        Graph prev_graph;
        Graph curr_graph;

        BulkSyncDispatcher(Graph &graph) {
            prev_graph = graph;
//            curr_graph = Graph::template FromEdgeListWithExecutor<Allocator>();
//            curr_graph.vertex_offsets = prev_graph.vertex_offsets;
//            curr_graph.vertex_data = prev_graph.vertex_data;
            curr_graph = Graph::Next(graph);

            //std::cout << "!!" << sizeof(_ID) << std::endl;
        }


//        template<class Result>
        string run() {
            Kernel kernel;
            const auto num_vertices = prev_graph.num_vertices;
            const auto num_edges = prev_graph.num_edges;

            for (ID src = 0; src < num_vertices; ++src) {
                for (ID i = 0, end = prev_graph.vertex_degrees[src]; i < end; ++i) {
                    auto dst = prev_graph.vertex_neighbors[src][i];
                    auto index = prev_graph.vertex_offsets[src] + i;
                    curr_graph.e_data[prev_graph.forward_indices[index]]
                            = kernel.scatter(src, dst, prev_graph.v_data[src]);
                }
            }
            for (ID src = 0; src < num_vertices; ++src) {
                for (ID i = 0, end = prev_graph.vertex_degrees[src]; i < end; ++i) {
                    auto dst = prev_graph.vertex_neighbors[src][i];
                    auto index = prev_graph.vertex_offsets[src] + i;
                    curr_graph.e_data[index] = kernel.gather(src, dst,
                                                          prev_graph.v_data[src],
                                                          curr_graph.e_data[index]);
                }
            }

            for (ID dst = 0; dst < num_vertices; ++dst) {
                for (ID i = 0, end = prev_graph.in_degrees[dst]; i < end; ++i) {
                    auto src = prev_graph.in_neighbors[dst][i];
                    auto index = prev_graph.in_offsets[dst] + i;
                    curr_graph.v_data[dst] = kernel.sum(dst, src,
                                                       prev_graph.v_data[dst],
                                                       curr_graph.e_data[index]);
                }
                curr_graph.v_data[dst] = kernel.apply(dst,
                                                      prev_graph.v_data[dst],
                                                      curr_graph.v_data[dst]);
            }


            std::cout << "dispatched" << std::endl;
            return "hoge";
        }
    };
}


#endif //HOSHIZORA_BULKSYNCDISPATCHER_H
