#ifndef HOSHIZORA_BULKSYNC_GAS_EXECUTOR_H
#define HOSHIZORA_BULKSYNC_GAS_EXECUTOR_H

#include <string>
#include <thread>

#include "hoshizora/core/executor/executor.h"
#include "hoshizora/core/util/bulksync_thread_pool.h"
#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/util/loop.h"

namespace hoshizora {
template <class Kernel> struct BulkSyncGASExecutor : Executor<Kernel> {
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

  explicit BulkSyncGASExecutor(Graph &graph, u32 num_iters)
      : prev_graph(&graph), curr_graph(&graph),
        num_vertices(graph.num_vertices), num_edges(graph.num_edges),
        thread_pool(num_threads), num_iters(num_iters) {
    curr_graph->set_v_data(true);
    curr_graph->set_e_data(true);
  }

  //template <class Func> inline void push_tasks(Func f, ID *boundaries) {
  //  auto tasks = new std::vector<std::function<void()>>();
  //  loop::each_thread(boundaries, [&](u32 thread_id, u32 numa_id, u32 lower,
  //                                    u32 upper, ID acc_num_srcs) {
  //    tasks->emplace_back([=, &f]() {
  //      for (ID i = lower; i < upper; ++i) {
  //        f(i, thread_id, numa_id);
  //      }
  //    });
  //  });
  //  thread_pool.push_tasks(tasks);
  //}

  template<class Func>
  inline void push_tasks(Func f, ID *boundaries) {
    auto tasks = new std::vector<std::function<void()>>();
    loop::each_thread(boundaries,
                      [&](u32 thread_id, u32 numa_id, u32 lower, u32 upper) {
                        tasks->emplace_back([=, &f]() {
                          for (ID dst = lower; dst < upper; ++dst) {
                            f(dst, thread_id);
                          }
                        });
                      });
    thread_pool.push_tasks(tasks);
  }

  template<class Func>
  inline void push_tasks(Func f, ID *boundaries, u32 iter) {
    auto tasks = new std::vector<std::function<void()>>();
    loop::each_thread(boundaries,
                      [&](u32 thread_id, u32 numa_id, u32 lower, u32 upper) {
                        tasks->emplace_back([=, &f]() {
                          for (ID dst = lower; dst < upper; ++dst) {
                            f(dst, thread_id);
                          }
                          if (thread_id == num_threads - 1) {
                            SPDLOG_DEBUG(debug::logger, "fin iter: {}", iter);
                          }
                        });
                      });
    thread_pool.push_tasks(tasks);
  }

  template <
      class
      Func /*(from, to, thread_id, numa_id, local_offset, local_idx, global_offset)*/>
  inline void push_tasks(Func f, ID *boundaries, u32 iter,
                         colle::DiscreteArray<u8> &indices) {
    auto tasks = new std::vector<std::function<void()>>();

    loop::each_thread(boundaries, [&](u32 thread_id, u32 numa_id, u32 lower,
                                      u32 upper, u32 acc_num_srcs) {
      const auto num_inner_vertices = upper - lower;
      tasks->emplace_back([=, &f, &indices]() {
        indices.foreach (thread_id, num_inner_vertices,
                         [=](ID dst, ID local_offset, ID _global_idx,
                             ID local_idx, ID global_offset) {
                           f(acc_num_srcs + _global_idx, dst, thread_id,
                             numa_id, local_offset, local_idx, global_offset);
                         });

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
        push_tasks(
            [kernel, prev_graph](ID src, u32 thread_id/*, u32 numa_id*/) {
              prev_graph->v_data(src, thread_id) =
                  kernel.init(src, *prev_graph);

              // for (ID i = 0, end = prev_graph->out_degrees[src]; i < end;
              // ++i) {
              //    const auto index = prev_graph->out_offsets[src] + i;
              //        prev_graph->e_data[index] = kernel.init(src,
              //        prev_graph);
              //}
            },
            prev_graph->out_boundaries);
      } else {
        thread_pool.push_task([prev_graph, curr_graph]() {
          Graph::Next(*prev_graph, *curr_graph);
        });
      }

      // scatter and gather
      //push_tasks(
      //    [kernel, prev_graph, curr_graph](ID src, ID dst, u32 thread_id,
      //                                     u32 numa_id, u32 local_offset,
      //                                     u32 local_idx, u32 global_offset) {
      //      const auto forwarded_index =
      //          prev_graph
      //              ->forward_indices[global_offset + local_offset + local_idx];
      //      curr_graph->e_data(forwarded_index /*, thread_id*/) = kernel.gather(
      //          src, dst, prev_graph->e_data(forwarded_index /*, thread_id*/),
      //          kernel.scatter(src, dst, prev_graph->v_data(src, thread_id),
      //                         *prev_graph),
      //          *prev_graph);
      //    },
      //    prev_graph->out_boundaries, iter, prev_graph->out_indices);

      // sum and apply
      //push_tasks(
      //    [kernel, prev_graph, curr_graph](ID dst, ID src, u32 thread_id,
      //                                     u32 numa_id, u32 local_offset,
      //                                     u32 local_idx, u32 global_offset) {
            // FIXME: each_dst
            //curr_graph->v_data(dst /*, thread_id*/) =
            //    kernel.zero(dst, *prev_graph);
            //
            //{
            //  const auto index = global_offset + local_offset + local_idx;
            //  curr_graph->v_data(dst /*, thread_id*/) = kernel.sum(
            //      dst, src, curr_graph->v_data(dst /*, thread_id*/),
            //      curr_graph->e_data(index /*, thread_id*/), *prev_graph);
            //}

            // FIXME: each_dst
            //curr_graph->v_data(dst /*, thread_id*/) = kernel.apply(
            //    dst, prev_graph->v_data(dst /*, thread_id*/),
            //    curr_graph->v_data(dst /*, thread_id*/), *prev_graph);
          //},
          //prev_graph->in_boundaries, iter, prev_graph->in_indices);


      // scatter and gather
      push_tasks([kernel, prev_graph, curr_graph](ID src, u32 thread_id) {
        for (ID i = 0, end = prev_graph->out_degrees(src, thread_id); i < end; ++i) {
          const auto dst = prev_graph->out_neighbors(src, thread_id)[i];
          const auto index = prev_graph->out_offsets(src, thread_id) + i;
          const auto forwarded_index = prev_graph->forward_indices[index];

          curr_graph->e_data(forwarded_index/*, thread_id*/)
              = kernel.gather(src, dst,
                              prev_graph->e_data(forwarded_index/*, thread_id*/),
                              kernel.scatter(src, dst, prev_graph->v_data(src, thread_id),
                                             *prev_graph),
                              *prev_graph);
        }
      }, prev_graph->out_boundaries);

      // sum and apply
      push_tasks([kernel, curr_graph, prev_graph](ID dst, u32 thread_id) {
        curr_graph->v_data(dst/*, thread_id*/) = kernel.zero(dst, *prev_graph); // TODO
        for (ID i = 0, end = prev_graph->in_degrees(dst, thread_id); i < end; ++i) {
          const auto src = prev_graph->in_neighbors(dst, thread_id)[i];
          const auto index = prev_graph->in_offsets(dst, thread_id) + i;

          curr_graph->v_data(dst/*, thread_id*/) = kernel.sum(dst, src,
                                                            curr_graph->v_data(dst/*, thread_id*/),
                                                            curr_graph->e_data(index/*, thread_id*/),
                                                            *prev_graph);
        }

        curr_graph->v_data(dst/*, thread_id*/) = kernel.apply(dst,
                                                            prev_graph->v_data(dst/*, thread_id*/),
                                                            curr_graph->v_data(dst/*, thread_id*/),
                                                            *prev_graph);
      }, prev_graph->in_boundaries, iter);
    }

    thread_pool.quit();

    return kernel.result(*curr_graph);
  }
};
} // namespace hoshizora

#endif // HOSHIZORA_BULKSYNC_GAS_EXECUTOR_H
