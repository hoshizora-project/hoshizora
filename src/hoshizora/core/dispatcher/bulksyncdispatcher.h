#ifndef HOSHIZORA_BULKSYNCDISPATCHER_H
#define HOSHIZORA_BULKSYNCDISPATCHER_H

#include <string>
#include "hoshizora/core/dispatcher/dispatcher.h"
#include <thread>
#include <hoshizora/core/util/bulksync_thread_pool.h>

namespace hoshizora {
    template<class Kernel, class Allocator>
    struct BulkSyncDispatcher : Dispatcher<Kernel, Allocator> {
        using Graph = typename Kernel::_Graph;
        using ID = typename Kernel::_Graph::_ID;

        Graph prev_graph;
        Graph curr_graph;

        const ID num_vertices;
        const ID num_edges;

        const u32 num_threads = thread::hardware_concurrency(); // TODO
        BulkSyncThreadPool thread_pool;

        explicit BulkSyncDispatcher(Graph &graph)
                : prev_graph(graph), curr_graph(graph), thread_pool(num_threads),
                  num_vertices(graph.num_vertices), num_edges(graph.num_edges) {
            curr_graph.set_v_data();
            curr_graph.set_e_data();
        }


        template<class Func>
        void push_outbound(Func f) {
            std::vector<std::function<void(u32)>> bulk;
            for (u32 i = 0; i < num_threads; ++i) {
                bulk.emplace_back([&, i](u32 thread_id) {
                    for (ID src = prev_graph.out_boundaries[i]; src < prev_graph.out_boundaries[i + 1]; ++src) {
                        f(src, mock::thread_to_numa(thread_id));
                    }
                });
            }
            thread_pool.push_bulk(bulk);
        }

        template<class Func>
        void push_inbound(Func f) {
            std::vector<std::function<void(u32)>> bulk;
            for (u32 i = 0; i < num_threads; ++i) {
                bulk.emplace_back([&, i](u32 thread_id) {
                    for (ID dst = prev_graph.in_boundaries[i]; dst < prev_graph.in_boundaries[i + 1]; ++dst) {
                        f(dst, mock::thread_to_numa(thread_id));
                    }
                });
            }
            thread_pool.push_bulk(bulk);
        }

        std::string run() {
            auto kernel = Kernel();

            constexpr auto num_iters = 2;
            for (auto iter = 0u; iter < num_iters; ++iter) {
                if (iter == 0) {
                    push_outbound([&](ID src, u32 numa_id) {
                        prev_graph.v_data(src, numa_id) = kernel.init(src, prev_graph);

                        /*
                        for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                            const auto index = prev_graph.out_offsets[src] + i;
//                                prev_graph.e_data[index] = kernel.init(src, prev_graph);
                        }
                         */
                    });
                } else {
                    Graph::Next(prev_graph, curr_graph);
                }

                // scatter and gather
                push_outbound([&](ID src, u32 numa_id) {
                    for (ID i = 0, end = prev_graph.out_degrees(src, numa_id); i < end; ++i) {
                        const auto dst = prev_graph.out_neighbors(src, numa_id)[i];
                        const auto index = prev_graph.out_offsets(src, numa_id) + i;
                        const auto forwarded_index = prev_graph.forward_indices[index];

                        curr_graph.e_data(forwarded_index, numa_id)
                                = kernel.gather(src, dst,
                                                prev_graph.e_data(forwarded_index, numa_id),
                                                kernel.scatter(src, dst, prev_graph.v_data(src, numa_id), prev_graph),
                                                prev_graph);
                    }
                });

                // sum and apply
                push_inbound([&](ID dst, u32 numa_id) {
                    curr_graph.v_data(dst, numa_id) = kernel.zero(dst, prev_graph); // TODO
                    for (ID i = 0, end = prev_graph.in_degrees(dst, numa_id); i < end; ++i) {
                        const auto src = prev_graph.in_neighbors(dst, numa_id)[i];
                        const auto index = prev_graph.in_offsets(dst, numa_id) + i;

                        curr_graph.v_data(dst, numa_id) = kernel.sum(dst, src,
                                                                     curr_graph.v_data(dst, numa_id),
                                                                     curr_graph.e_data(index, numa_id),
                                                                     prev_graph);
                    }

                    curr_graph.v_data(dst, numa_id) = kernel.apply(dst,
                                                                   prev_graph.v_data(dst, numa_id),
                                                                   curr_graph.v_data(dst, numa_id),
                                                                   prev_graph);
                });
            }

            thread_pool.terminate_if_empty();

            return kernel.result(curr_graph);
        }
    };
}


#endif //HOSHIZORA_BULKSYNCDISPATCHER_H
