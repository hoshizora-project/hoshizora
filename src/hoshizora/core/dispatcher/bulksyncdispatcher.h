#ifndef HOSHIZORA_BULKSYNCDISPATCHER_H
#define HOSHIZORA_BULKSYNCDISPATCHER_H

#include <string>
#include <thread>
#include "hoshizora/core/dispatcher/dispatcher.h"
#include "hoshizora/core/util/bulksync_thread_pool.h"

namespace hoshizora {
    template<class Kernel>
    struct BulkSyncDispatcher : Dispatcher<Kernel> {
        using Graph = typename Kernel::_Graph;
        using ID = typename Kernel::_Graph::_ID;

        Graph *prev_graph;
        Graph *curr_graph;

        const ID num_vertices;
        const ID num_edges;

        Kernel kernel;

        // TODO
        const u32 num_threads = loop::num_threads;
        BulkSyncThreadPool thread_pool;

        const u32 num_iters;

        explicit BulkSyncDispatcher(Graph &graph, u32 num_iters)
                : prev_graph(&graph), curr_graph(&graph),
                  num_vertices(graph.num_vertices), num_edges(graph.num_edges),
                  thread_pool(num_threads), num_iters(num_iters) {
            curr_graph->set_v_data(true);
            curr_graph->set_e_data(true);
        }

        template<class Func>
        inline void push_tasks(Func f, ID *boundaries) {
            auto tasks = new std::vector<std::function<void()>>();
            loop::each_thread(boundaries,
                              [&](u32 numa_id, u32 thread_id, u32 lower, u32 upper) {
                                  tasks->emplace_back([=, &f]() {
                                      for (ID dst = lower; dst < upper; ++dst) {
                                          f(dst, numa_id);
                                      }
                                  });
                              });
            thread_pool.push_tasks(tasks);
        }

        template<class Func>
        inline void push_tasks(Func f, ID *boundaries, u32 iter) {
            auto tasks = new std::vector<std::function<void()>>();
            loop::each_thread(boundaries,
                              [&](u32 numa_id, u32 thread_id, u32 lower, u32 upper) {
                                  tasks->emplace_back([=, &f]() {
                                      for (ID dst = lower; dst < upper; ++dst) {
                                          f(dst, numa_id);
                                      }
                                      if (thread_id == num_threads - 1) {
                                          SPDLOG_DEBUG(debug::logger, "fin iter: {}", iter);
                                      }
                                  });
                              });
            thread_pool.push_tasks(tasks);
        }

        std::string run() {
            for (auto iter = 0u; iter < num_iters; ++iter) {
                SPDLOG_DEBUG(debug::logger, "push iter: {}", iter);
                auto kernel = this->kernel; // FIXME
                auto prev_graph = this->prev_graph;
                auto curr_graph = this->curr_graph;

                if (iter == 0) {
                    push_tasks([kernel, prev_graph](ID src, u32 numa_id) {
                        prev_graph->v_data(src, numa_id) = kernel.init(src, *prev_graph);

                        //for (ID i = 0, end = prev_graph->out_degrees[src]; i < end; ++i) {
                        //    const auto index = prev_graph->out_offsets[src] + i;
                        //        prev_graph->e_data[index] = kernel.init(src, prev_graph);
                        //}
                    }, prev_graph->out_boundaries);
                } else {
                    thread_pool.push_task([prev_graph, curr_graph]() {
                        Graph::Next(*prev_graph, *curr_graph);
                    });
                }

                // scatter and gather
                push_tasks([kernel, prev_graph, curr_graph](ID src, u32 numa_id) {
                    for (ID i = 0, end = prev_graph->out_degrees(src, numa_id); i < end; ++i) {
                        const auto dst = prev_graph->out_neighbors(src, numa_id)[i];
                        const auto index = prev_graph->out_offsets(src, numa_id) + i;
                        const auto forwarded_index = prev_graph->forward_indices[index];

                        curr_graph->e_data(forwarded_index/*, numa_id*/)
                                = kernel.gather(src, dst,
                                                prev_graph->e_data(forwarded_index/*, numa_id*/),
                                                kernel.scatter(src, dst, prev_graph->v_data(src, numa_id),
                                                               *prev_graph),
                                                *prev_graph);
                    }
                }, prev_graph->out_boundaries);

                // sum and apply
                push_tasks([kernel, curr_graph, prev_graph](ID dst, u32 numa_id) {
                    curr_graph->v_data(dst/*, numa_id*/) = kernel.zero(dst, *prev_graph); // TODO
                    for (ID i = 0, end = prev_graph->in_degrees(dst, numa_id); i < end; ++i) {
                        const auto src = prev_graph->in_neighbors(dst, numa_id)[i];
                        const auto index = prev_graph->in_offsets(dst, numa_id) + i;

                        curr_graph->v_data(dst/*, numa_id*/) = kernel.sum(dst, src,
                                                                          curr_graph->v_data(dst/*, numa_id*/),
                                                                          curr_graph->e_data(index/*, numa_id*/),
                                                                          *prev_graph);
                    }

                    curr_graph->v_data(dst/*, numa_id*/) = kernel.apply(dst,
                                                                        prev_graph->v_data(dst/*, numa_id*/),
                                                                        curr_graph->v_data(dst/*, numa_id*/),
                                                                        *prev_graph);
                }, prev_graph->in_boundaries, iter);
            }

            thread_pool.quit();

            return kernel.result(*curr_graph);
        }
    };
}


#endif //HOSHIZORA_BULKSYNCDISPATCHER_H
