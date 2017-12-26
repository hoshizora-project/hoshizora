#ifndef HOSHIZORA_LOOP_H
#define HOSHIZORA_LOOP_H

#include "hoshizora/core/util/colle.h"
#include "hoshizora/core/util/includes.h"

namespace hoshizora::loop {
// ThreadPool pool;

// static void quit() { pool.quit(); }

template <
    class
    Func /*(thread_id, numa_id, dst, global_idx, local_idx, global_offset)*/>
static inline void
each_index(const u32 *const boundaries,
           const colle::DiscreteArray<u8> &compressed_indices,
           const colle::DiscreteArray<u32> &offsets, Func f) {

  auto acc_num_srcs = 0;
  for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
    const auto numa_id = mock::thread_to_numa(thread_id);
    const auto lower = boundaries[thread_id];
    const auto upper = boundaries[thread_id + 1];
    const auto num_inner_vertices = upper - lower;

    compressed_indices.foreach (
        thread_id, num_inner_vertices,
        [=, &f](u32 i, u32 local_offset, u32 _global_idx, u32 local_idx,
                u32 global_offset) {
          f(thread_id, numa_id, i, local_offset, acc_num_srcs + _global_idx,
            local_idx, global_offset);
        });
    acc_num_srcs += num_inner_vertices;
  }
}

template<class Func>
static inline void each_thread(const u32 *const boundaries, Func f) {
  for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
    const auto numa_id = mock::thread_to_numa(thread_id);
    //f(numa_id, thread_id, boundaries[thread_id], boundaries[thread_id + 1]);
    f(thread_id, numa_id, boundaries[thread_id], boundaries[thread_id + 1]);
  }
}

//template <class Func /*(thread_id, numa_id, lower, upper, acc_num_srcs)*/>
//static inline void each_thread(const u32 *const boundaries, Func f) {
//  auto acc_num_srcs = 0;
//  for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
//    const u32 numa_id = mock::thread_to_numa(thread_id);
//    const auto lower = boundaries[thread_id];
//    const auto upper = boundaries[thread_id + 1];
//    const auto num_inner_vertices = upper - lower;
//
//    f(thread_id, numa_id, lower, upper, acc_num_srcs);
//    acc_num_srcs += num_inner_vertices;
//  }
//}

/*
template <class Func>
static inline void each_thread0(const u32 *const boundaries, Func f) {
  auto tasks = new std::vector<std::function<void()>>();
  for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
    tasks->emplace_back([&, thread_id]() {
      const auto numa_id = mock::thread_to_numa(thread_id);
      // require thread-safe function
      f(numa_id, thread_id, boundaries[thread_id], boundaries[thread_id + 1]);
    });
  }
  pool.push_tasks(tasks);
}
 */

template<class Func>
static inline void each_numa(const u32 *const boundaries, Func f) {
  u32 numa_id = 0;
  u32 lower = boundaries[0];

  for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
    if (thread_id == num_threads - 1 || numa_id != mock::thread_to_numa(thread_id + 1)) {
      f(numa_id, lower, boundaries[thread_id + 1]);

      numa_id = mock::thread_to_numa(thread_id + 1);
      lower = boundaries[thread_id + 1];
    }
  }
}
} // namespace hoshizora::loop
#endif // HOSHIZORA_LOOP_H
