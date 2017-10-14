#ifndef HOSHIZORA_CONDITION_VARIABLE_BARRIER_H
#define HOSHIZORA_CONDITION_VARIABLE_BARRIER_H

#include <mutex>
#include <condition_variable>
#include "hoshizora/core/util/includes.h"

namespace hoshizora {
    class condition_variable_barrier {
    private:
        const u32 num_threads;
        std::atomic<u32> num_waits;
        std::atomic<bool> sense;
        std::vector<u32> local_sense;
        std::mutex mtx;
        std::condition_variable cond;

        inline u32 tid2idx(u32 i) { return i * 64 / sizeof(u32); }

    public:
        condition_variable_barrier() = delete;

        condition_variable_barrier(const condition_variable_barrier &) = delete;

        condition_variable_barrier &operator=(const condition_variable_barrier &) = delete;

        explicit condition_variable_barrier(u32 num_threads) :
                num_threads(num_threads),
                num_waits(num_threads),
                sense(false),
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
                cond.notify_all();
            } else {
                std::unique_lock<std::mutex> ul(mtx);
                cond.wait(ul, [&]() { return _sense == sense; });
                SPDLOG_DEBUG(debug::logger, "wakedup[{}]", thread_id);
            }

            // reset
            local_sense[tid2idx(thread_id)] = static_cast<u32>(!_sense);
        }
    };
}

#endif //HOSHIZORA_CONDITION_VARIABLE_BARRIER_H
