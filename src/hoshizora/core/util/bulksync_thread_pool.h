#ifndef HOSHIZORA_THREAD_POOL_H
#define HOSHIZORA_THREAD_POOL_H

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <hoshizora/core/util/includes.h>
#include <assert.h>

namespace hoshizora {
    struct BulkSyncThreadPool {
        std::vector<std::thread> pool;
        std::vector<std::queue<std::function<void()>>> tasks;
        std::vector<bool> sync_flags;
        bool terminate_flag = false;
        u32 num_threads = 0;

        inline void flipWait(u32 n) {
            sync_flags[n] = !sync_flags[n];
            while (true) {
                auto breakable = true;
                for (const auto &flag: sync_flags) {
                    breakable = flag == sync_flags[n];
                }
                if (breakable) break;
            }
        }

        BulkSyncThreadPool(u32 num_threads) : num_threads(num_threads) {
            for (auto n = 0u; n < num_threads; ++n) {
                std::queue<std::function<void()>> queue;
                sync_flags.emplace_back(false);
                tasks.emplace_back(queue);
            }

            for (auto n = 0u; n < num_threads; ++n) {
                pool.emplace_back(std::thread([&, n]() {
                    // TODO: thread affinity
                    debug::print("created");

                    while (!terminate_flag) {
                        if (tasks[n].empty()) continue;

                        const auto &task = tasks[n].front();
                        task();
                        tasks[n].pop();
                        flipWait(n);
                    }
                }));
            }
        }

        void push_bulk(std::vector<std::function<void()>> &bulk) {
            assert(num_threads == bulk.size());

            for (u32 n = 0; n < num_threads; ++n) {
                tasks[n].push(std::move(bulk[n]));
            }
        }

        void terminate() {
            terminate_flag = true;
            for (auto &thread: pool) {
                thread.join();
            }
        }

        void terminate_if_empty() {

            for (auto &thread: pool) {
                thread.join();
            }
        }
    };
}


#endif //HOSHIZORA_THREAD_POOL_H
