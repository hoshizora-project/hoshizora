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
            std::vector<std::function<void()>> bulk;
            for (u32 i = 0; i < num_threads; ++i) {
                bulk.emplace_back([&, i]() {
                    for (ID src = prev_graph.out_boundaries[i]; src < prev_graph.out_boundaries[i + 1]; ++src) {
                        f(src);
                    }
                });
            }
            thread_pool.push_bulk(bulk);
        }

        template<class Func>
        void push_inbound(Func f) {
            std::vector<std::function<void()>> bulk;
            for (u32 i = 0; i < num_threads; ++i) {
                bulk.emplace_back([&, i]() {
                    for (ID dst = prev_graph.in_boundaries[i]; dst < prev_graph.in_boundaries[i + 1]; ++dst) {
                        f(dst);
                    }
                });
            }
            thread_pool.push_bulk(bulk);
        }

        std::string run() {
            Kernel kernel;

            // TODO
            const auto _prev_graph = prev_graph;
            const auto _curr_graph = curr_graph;

            constexpr auto num_iters = 2;
            for (auto iter = 0u; iter < num_iters; ++iter) {
                if (iter == 0) {
                    push_outbound([&, _prev_graph](ID src) {
                        _prev_graph.v_data[src] = kernel.init(src, _prev_graph);

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
                push_outbound([&, _prev_graph, _curr_graph](ID src) {
                    for (ID i = 0, end = _prev_graph.out_degrees[src]; i < end; ++i) {
                        const auto dst = _prev_graph.out_neighbors[src][i];
                        const auto index = _prev_graph.out_offsets[src] + i;
                        const auto forwarded_index = _prev_graph.forward_indices[index];

                        _curr_graph.e_data[forwarded_index]
                                = kernel.gather(src, dst,
                                                _prev_graph.e_data[forwarded_index],
                                                kernel.scatter(src, dst, _prev_graph.v_data[src], _prev_graph),
                                                _prev_graph);
                    }
                });

                // sum and apply
                push_inbound([&, _prev_graph, _curr_graph](ID dst) {
                    _curr_graph.v_data[dst] = kernel.zero(dst, _prev_graph); // TODO
                    for (ID i = 0, end = _prev_graph.in_degrees[dst]; i < end; ++i) {
                        const auto src = _prev_graph.in_neighbors[dst][i];
                        const auto index = _prev_graph.in_offsets[dst] + i;

                        _curr_graph.v_data[dst] = kernel.sum(dst, src,
                                                            _curr_graph.v_data[dst],
                                                            _curr_graph.e_data[index], _prev_graph);
                    }
                    _curr_graph.v_data[dst] = kernel.apply(dst,
                                                          _prev_graph.v_data[dst],
                                                          _curr_graph.v_data[dst], _prev_graph);
                });
            }

            thread_pool.terminate_if_empty();

            return kernel.result(curr_graph);
        }
    };
}


#endif //HOSHIZORA_BULKSYNCDISPATCHER_H
