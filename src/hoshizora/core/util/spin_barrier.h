#ifndef HOSHIZORA_SPIN_BARRIER_H
#define HOSHIZORA_SPIN_BARRIER_H

#include "hoshizora/core/util/includes.h"

namespace hoshizora {
class spin_barrier {
private:
  const u32 num_threads;
  std::atomic<u32> num_waits;
  std::atomic<bool> sense;
  std::vector<u32> local_sense;

  inline u32 tid2idx(u32 i) { return i * 64 / sizeof(u32); }

public:
  spin_barrier() = delete;

  spin_barrier(const spin_barrier &) = delete;

  spin_barrier &operator=(const spin_barrier &) = delete;

  explicit spin_barrier(u32 num_threads)
      : num_threads(num_threads), num_waits(num_threads), sense(false),
        local_sense(tid2idx(num_threads)) {
    for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
      local_sense[tid2idx(thread_id)] = static_cast<u32>(true);
    }
  }

  void wait(u32 thread_id) {
    int _sense = local_sense[tid2idx(thread_id)];

    SPDLOG_DEBUG(debug::logger, "wait[{}]", thread_id);
    if (num_waits.fetch_sub(1) == 1) {
      SPDLOG_DEBUG(debug::logger, "wakeup[{}]", thread_id);

      // reset
      num_waits.store(num_threads);
      sense.store(!sense, std::memory_order_release);
    } else {
      while (_sense != sense) {
      }
      SPDLOG_DEBUG(debug::logger, "wakedup[{}]", thread_id);
    }

    // reset
    local_sense[tid2idx(thread_id)] = static_cast<u32>(!_sense);
  }
};
} // namespace hoshizora

#endif // HOSHIZORA_SPIN_BARRIER_H
