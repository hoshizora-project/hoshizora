#ifndef HOSHIZORA_INCLUDES_H
#define HOSHIZORA_INCLUDES_H

#include <cstdint>
#include <cstdlib>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <thread>
#include <string>
#include <iostream>
#ifdef __linux__
#include <sched.h>
#elif __APPLE__
#include <mach/thread_act.h>
#endif
#include "hoshizora/core/util/primitive_includes.h"
#include "hoshizora/core/util/thread_pool.h"
#include "hoshizora/core/util/numa_allocator.h"

namespace hoshizora {
    template<class T>
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
            for(u32 numa_id = 0; numa_id < loop::num_numa_nodes; ++numa_id) {
                allocators.emplace_back(new NumaAllocator<u32 *>(numa_id));
            }
        }
#endif
    }

    namespace colle {
        template<class T>
        static inline numa_vector<T> *make_numa_vector(u32 numa_id) {
#ifdef SUPPORT_NUMA
            return new numa_vector<T>(*(mem::allocators[numa_id]));
#else
            return new numa_vector<T>();
#endif
        }

        // TODO: SIMD-aware
        template<class T>
        struct DiscreteArray {
            // TODO: Redundant on each numa node
            std::vector<T *> data;
            std::vector<u32> range;

            DiscreteArray() {
                range.emplace_back(0);
            }

            DiscreteArray(std::vector<T *> &data, std::vector<u32> &range)
                    : data(data), range(range) {}

            ~DiscreteArray() {
                //debug::print("freed disc malloc"+std::to_string(range.size()));
            }

            // Though like copy constructor, this makes only references numa-local
            DiscreteArray<T> redundant() {
                // TODO: need numa allocator
                auto data = std::vector<T *>();
                auto range = std::vector<u32>();
                std::copy(begin(this->data), end(this->data), std::back_inserter(data));
                std::copy(begin(this->range), end(this->range), std::back_inserter(range));
                return DiscreteArray<T>(data, range);
            }

            /*
            const T &operator[](u32 index) const {
                const auto n = std::distance(begin(range) + 1,
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }*/

            /*
            T &operator[](u32 index) &{
                const auto n = std::distance(begin(range) + 1,
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }
             */

            /*
            T operator[](u32 index) &&{
                const auto n = std::distance(begin(range) + 1,
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }*/

            /*
            T &operator[](u32 index, u32 n) {
                return data[n][index - range[n]];
            }
             */

            void add(T *datum, u32 length) {
                data.emplace_back(datum);
                range.emplace_back(range.back() + length);
            }

            void add(numa_vector<T> &chunk) {
                data.emplace_back(chunk.data());
                range.emplace_back(range.back() + chunk.size());
            }

            void add(numa_vector<T> &&chunk) {
                data.emplace_back(chunk.data());
                range.emplace_back(range.back() + chunk.size());
            }

            void mod_last(u32 length) {
                const auto size = range.size();
                if (size == 1) return; // no contents

                range[size - 1] = range[size - 2] + length;
            }

            // significantly slower than normal index access on a single malloc
            //[[deprecated("Recommended to call with hint")]]
            T &operator()(u32 index) {
                // TODO: sequential search may be faster
                const auto n = std::distance(begin(range) + 1,
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }

            T &operator()(u32 index, void *dummy) const {
                // TODO: sequential search may be faster
                const auto n = std::distance(begin(range) + 1,
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }

            // TODO
            // faster than normal index access on a single malloc
            T operator()(u32 index, u32 n, u32 dummy) const {
                // if constexpr (support_numa) data[n][index - range[n]] else data[0][index];
                return data[n][index - range[n]];
            }

            // TODO
            // faster than normal index access on a single malloc
            T &operator()(u32 index, u32 n) {
                // if constexpr (support_numa) data[n][index - range[n]] else data[0][index];
                return data[n][index - range[n]];
            }

            // TODO
            // significantly faster than normal index access on a single malloc
            /*
            T &operator()(u32 index, u32 n, u32 dummy) {
                return data[n][local_index];
            }
             */
        };
    }

    namespace loop {
        ThreadPool pool;

        static void quit() {
            pool.quit();
        }

        template<class Func>
        static inline void each_index(const u32 *const boundaries, Func f) {
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                const auto numa_id = mock::thread_to_numa(thread_id);
                for (u32 i = boundaries[thread_id], end = boundaries[thread_id + 1]; i < end; ++i) {
                    f(numa_id, thread_id, i);
                }
            }
        }

        template<class Func>
        static inline void each_thread(const u32 *const boundaries, Func f) {
            u32 block_id = 0; // e.g., numa [0,1,2,3,0,1,2,3] -> block [0,1,2,3,4,5,6,7]
            u32 numa_id = mock::thread_to_numa(0);
            u32 lower = boundaries[0];

            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                f(block_id, numa_id, thread_id, lower, boundaries[thread_id + 1]);

                // go next
                lower = boundaries[thread_id + 1];
                if (thread_id != num_threads - 1 && numa_id != mock::thread_to_numa(thread_id + 1)) {
                    block_id++;
                    numa_id = mock::thread_to_numa(thread_id + 1);
                }
            }
        }

        template<class Func>
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

        template<class Func>
        static inline void each_numa_node(const u32 *const boundaries, Func f) {
            u32 block_id = 0; // e.g., numa [0,1,2,3,0,1,2,3] -> block [0,1,2,3,4,5,6,7]
            u32 numa_id = mock::thread_to_numa(0);
            u32 lower = boundaries[0];

            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                if (thread_id == num_threads - 1 || numa_id != mock::thread_to_numa(thread_id + 1)) {
                    f(block_id, numa_id, lower, boundaries[thread_id + 1]);

                    // go next
                    block_id++;
                    numa_id = mock::thread_to_numa(thread_id + 1);
                    lower = boundaries[thread_id + 1];
                }
            }
        }

        template<class Func>
        static inline void each_numa_node0(const u32 *const boundaries, Func f) {
            auto tasks = new std::vector<std::function<void()>>();
            u32 numa_id = mock::thread_to_numa(0);
            u32 lower = boundaries[0];

            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                if (thread_id == num_threads - 1 || numa_id != mock::thread_to_numa(thread_id + 1)) {
                    const auto upper = boundaries[thread_id + 1];
                    tasks->emplace_back([&, numa_id, thread_id, lower, upper]() {
                        f(numa_id, lower, upper); // require thread-safe function
                    });

                    numa_id = mock::thread_to_numa(thread_id + 1);
                    lower = boundaries[thread_id + 1];
                } else {
                    tasks->emplace_back([]() {}); // FIXME
                }
            }

            pool.push_tasks(tasks);
        }

        template<class Func>
        static inline void each_numa_node(const u32 *const boundaries,
                                          const colle::DiscreteArray<u32> &offsets,
                                          Func f) {
            u32 block_id = 0;
            u32 numa_id = mock::thread_to_numa(0);
            u32 lower = boundaries[0];

            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                if (thread_id == num_threads - 1 || numa_id != mock::thread_to_numa(thread_id + 1)) {
                    const auto start = offsets(lower, block_id, 0);
                    const auto end = thread_id != num_threads - 1 // not last block
                                     ? offsets(boundaries[thread_id + 1], block_id + 1, 0)
                                     : offsets(boundaries[thread_id + 1], block_id, 0); // w/ cap

                    f(block_id, numa_id, start, end);

                    block_id++;
                    numa_id = mock::thread_to_numa(thread_id + 1);
                    lower = boundaries[thread_id + 1];
                }
            }
        }
    }

    static inline void init() {
        debug::init_logger();
#ifdef SUPPORT_NUMA
        mem::init_allocators();
#endif
    }
}

#endif //HOSHIZORA_INCLUDES_H
