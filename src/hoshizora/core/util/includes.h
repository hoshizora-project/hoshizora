#ifndef HOSHIZORA_INCLUDES_H
#define HOSHIZORA_INCLUDES_H

#include <cassert>
#include <condition_variable>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#ifdef __linux__
#include <sched.h>
#elif __APPLE__
#include <mach/thread_act.h>
#endif
#include "hoshizora/core/util/numa_allocator.h"
#include "hoshizora/core/util/primitive_includes.h"
#include "hoshizora/core/util/thread_pool.h"

namespace hoshizora {
template <class T>
using numa_vector =
#ifdef SUPPORT_NUMA
    std::vector<T, NumaAllocator<T>>;
#else
    std::vector<T>;
#endif

namespace mem {
#ifdef SUPPORT_NUMA
// TODO
static std::vector<NumaAllocator<u32 *> *> allocators;
static inline void init_allocators() {
  for (u32 numa_id = 0; numa_id < loop::num_numa_nodes; ++numa_id) {
    allocators.emplace_back(new NumaAllocator<u32 *>(numa_id));
  }
}
#endif
}

static inline void init() {
  debug::init_logger();
#ifdef SUPPORT_NUMA
  mem::init_allocators();
#endif
}
} // namespace hoshizora

#endif // HOSHIZORA_INCLUDES_H
