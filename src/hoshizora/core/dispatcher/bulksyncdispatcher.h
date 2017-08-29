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

        /*2,3
        u32 num_threads = thread::hardware_concurrency();
        std::vector<std::function<void()>> bulk;
        BulkSyncThreadPool thread_pool;
         */

//        vector<u32> out_boundaries;
//        vector<u32> in_boundaries;

        explicit BulkSyncDispatcher(Graph &graph)
                : prev_graph(graph), curr_graph(graph),
                  num_vertices(graph.num_vertices), num_edges(graph.num_edges) {
            curr_graph.set_v_data();
            curr_graph.set_e_data();

//            out_boundaries = {0, 100, 200, 350, num_vertices - 1}; // 仮
//            in_boundaries = {0, 90, 150, 400, num_vertices - 1}; // 仮
        }

        /*3
        explicit BulkSyncDispatcher(Graph &graph)
                : prev_graph(graph), curr_graph(graph), thread_pool(num_threads),
                  num_vertices(graph.num_vertices), num_edges(graph.num_edges) {
            curr_graph.set_v_data();
            curr_graph.set_e_data();

            bulk.reserve(num_threads);
            for (u32 i = 0; i < num_threads; ++i) {
                bulk.emplace_back(nullptr);
            }
        }
         */

        /*2
        template<class Func>
        void push(Func f) {
            for (u32 i = 0; i < num_threads; ++i) {
                bulk[i] = [&, i]() {
                    for (ID src = out_boundaries[i]; src < out_boundaries[i + 1]; ++src) {
                        f(src);
                    }
                    this_thread::sleep_for(std::chrono::seconds(i));
                    debug::print(this_thread::get_id());
                };
            }
            thread_pool.push_bulk(bulk);
        }
         */

        std::string run() {
            Kernel kernel;

            u32 num_threads = thread::hardware_concurrency();
            std::vector<std::function<void()>> bulk(4, []() {});
            BulkSyncThreadPool thread_pool(num_threads);


            // TODO: それぞれ次数の累積から求める, len == num_threads + 1
            vector<u32> out_boundaries = {0, 100, 200, 350, num_vertices - 1}; // 仮
            vector<u32> in_boundaries = {0, 90, 150, 400, num_vertices - 1}; // 仮

            constexpr auto num_iters = 2u;
            for (auto iter = 0u; iter < num_iters; ++iter) {
                if (iter == 0) {

//                    /*1
                    for (u32 i = 0; i < num_threads; ++i) {
                        bulk[i] = [&, i]() {
                            for (ID src = out_boundaries[i]; src < out_boundaries[i + 1]; ++src) {
                                prev_graph.v_data[src] = kernel.init(src, prev_graph);

                                for (ID j = 0, end = prev_graph.out_degrees[src]; j < end; ++j) {
                                    const auto index = prev_graph.out_offsets[src] + j;
//                                prev_graph.e_data[index] = kernel.init(src, prev_graph);
                                }
                            }
                        };
                    }
                    thread_pool.push_bulk(bulk);
                    debug::print("");
//                     */

                    /*3
                    for (u32 i = 0; i < num_threads; ++i) {
                        bulk[i] = [&, i]() {
                            for (ID src = out_boundaries[i]; src < out_boundaries[i + 1]; ++src) {
                                prev_graph.v_data[src] = kernel.init(src, prev_graph);

                                for (ID j = 0, end = prev_graph.out_degrees[src]; j < end; ++j) {
                                    const auto index = prev_graph.out_offsets[src] + j;
//                                prev_graph.e_data[index] = kernel.init(src, prev_graph);
                                }
                            }
                        };
                    }
                    thread_pool.push_bulk(bulk);
                    debug::print("");
                    */

                    /*2
                   push([&](ID src) {
                       prev_graph.v_data[src] = kernel.init(src, prev_graph);

                       for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                           const auto index = prev_graph.out_offsets[src] + i;
//                                prev_graph.e_data[index] = kernel.init(src, prev_graph);
                       }
                   });
                    */

                    // init
                    /*
                    for (ID src = 0; src < num_vertices; ++src) {
                        prev_graph.v_data[src] = kernel.init(src, prev_graph);

                        for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                            const auto index = prev_graph.out_offsets[src] + i;
//                            prev_graph.e_data[index] = kernel.init(src, prev_graph);
                        }
                    }
                     */
                } else {
                    Graph::Next(prev_graph, curr_graph);
                }



                // scatter and gather
                for (ID src = 0; src < num_vertices; ++src) {
                    for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                        const auto dst = prev_graph.out_neighbors[src][i];
                        const auto index = prev_graph.out_offsets[src] + i;
                        const auto forwarded_index = prev_graph.forward_indices[index];

                        curr_graph.e_data[forwarded_index]
                                = kernel.gather(src, dst,
                                                prev_graph.e_data[forwarded_index],
                                                kernel.scatter(src, dst, prev_graph.v_data[src], prev_graph),
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



//            vector<function<void()>> bulk;
//            for (u32 x = 0; x < 4; ++x) {
//                bulk.emplace_back([]() {
//                    debug::print(this_thread::get_id());
//                });
//            }
//            this_thread::sleep_for(std::chrono::seconds(2));
            for (u32 i = 0; i < num_threads; ++i) {
                bulk[i] = nullptr;
            }
            thread_pool.terminate();
            debug::print("fin");

/*
            for (u32 n = 0; n < num_threads; ++n) {
                threads.emplace_back(thread([=]() {
                    // scatter and gather
                    for (ID src = out_boundaries[n]; src < out_boundaries[n+1]; ++src) {
                        for (ID i = 0, end = prev_graph.out_degrees[src]; i < end; ++i) {
                            const auto dst = prev_graph.out_neighbors[src][i];
                            const auto index = prev_graph.out_offsets[src] + i;
                            const auto forwarded_index = prev_graph.forward_indices[index];

                            curr_graph.e_data[forwarded_index]
                                    = kernel.gather(src, dst,
                                                    prev_graph.e_data[forwarded_index],
                                                    kernel.scatter(src, dst, prev_graph.v_data[src], prev_graph),
                                                    prev_graph);
                        }
                    }


                    // sum and apply
                    for (ID dst = in_boundaries[n]; dst < in_boundaries[n+1]; ++dst) {
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

                }));
            }

            for (auto &thread: threads) {
                thread.join();
            }
*/


            return kernel.result(curr_graph);
        }
    };
}


#endif //HOSHIZORA_BULKSYNCDISPATCHER_H
