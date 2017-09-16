#ifndef HOSHIZORA_THREAD_POOL_H
#define HOSHIZORA_THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <assert.h>
#include <hoshizora/core/util/includes.h>

namespace hoshizora {
    class spin_barrier {
    public:
        spin_barrier() = delete;

        spin_barrier(const spin_barrier &) = delete;

        spin_barrier &operator=(const spin_barrier &) = delete;

        explicit spin_barrier(u32 num_threads) :
                num_threads(num_threads),
                num_waits(num_threads),
                sense(false),
                local_sense(tid2idx(num_threads)) {
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                local_sense[tid2idx(thread_id)] = static_cast<u32>(true);
            }
        }

        void wait(u32 i) {
            int _sense = local_sense[tid2idx(i)];
            debug::print("wait(" + std::to_string(i) + ")");

            if (num_waits.fetch_sub(1) == 1) {
                debug::print("wakeup(" + std::to_string(i) + ")");

                // reset
                num_waits.store(num_threads);
                sense.store(!sense, std::memory_order_release);
            } else {
                while (_sense != sense);
                debug::print("wakedup(" + std::to_string(i) + ")");
            }

            // reset
            local_sense[tid2idx(i)] = static_cast<u32>(!_sense);
        }

    private:
        const u32 num_threads;
        std::atomic<u32> num_waits;
        std::atomic<bool> sense;
        std::vector<u32> local_sense;

        inline u32 tid2idx(u32 i) { return i * 64 / sizeof(u32); }
    };

    struct BulkSyncThreadPool {
        std::vector<std::thread> pool;
        std::vector<std::queue<std::function<void()>>> tasks;
        bool force_quit_flag = false;
        bool quit_flag = false;
        u32 num_threads;
        std::mutex mtx; // TODO
        spin_barrier sb;

        explicit BulkSyncThreadPool(u32 num_threads)
                : num_threads(num_threads), sb(num_threads) {
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                std::queue<std::function<void()>> queue;
                tasks.emplace_back(queue);
            }

            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                pool.emplace_back(std::thread([&, thread_id]() {
                    // TODO: thread affinity
                    debug::print("created");

                    while (!force_quit_flag
                           && !(quit_flag && tasks[thread_id].empty())) {
                        if (tasks[thread_id].empty()) continue;

                        const auto &task = tasks[thread_id].front();
                        task();

                        mtx.lock();
                        debug::print("done(" + std::to_string(thread_id) + ")");
                        tasks[thread_id].pop(); // TODO: concurrent_queue
                        mtx.unlock();
                        sb.wait(thread_id);
                    }
                }));
            }
        }

        void push_bulk(std::vector<std::function<void()>> *bulk) {
            assert(num_threads == bulk->size());

            for (u32 n = 0; n < num_threads; ++n) {
                mtx.lock();
                tasks[n].push(std::move((*bulk)[n]));
                mtx.unlock();
            }
        }

        void force_quit() {
            force_quit_flag = true;
            for (auto &thread: pool) {
                thread.join();
            }
        }

        void quit() {
            quit_flag = true;
            for (auto &thread: pool) {
                thread.join();
            }
        }
    };
}


#endif //HOSHIZORA_THREAD_POOL_H
