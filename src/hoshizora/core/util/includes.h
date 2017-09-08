#ifndef HOSHIZORA_INCLUDES_H
#define HOSHIZORA_INCLUDES_H

#include <cstdint>
#include <stdlib.h>
#include <vector>
#include <cassert>
#include <thread>
#include "hoshizora/core/util/includes.h"

namespace hoshizora {
    using u8 = uint8_t;
    using u16 = uint16_t;
    using u32 = uint32_t;
    using u64 = uint64_t;
    using i8 = int8_t;
    using i16 = int16_t;
    using i32 = int32_t;
    using i64 = int64_t;
    using f32 = float;
    using f64 = double;
    using skip_t = std::nullptr_t[0];

    namespace debug {
        template<class T>
        static inline void print(T const &el) {
            std::cout << el << std::endl;
        }
    }

    namespace heap {
        template<class Type>
        static inline Type *array(u64 length) {
            return static_cast<Type *>(malloc(sizeof(Type) * length));
        }

        template<class Type>
        static inline Type *array0(u64 length) {
            auto arr = array<Type>(length);
            // TODO
            for (size_t i = 0; i < length; ++i) {
                arr[i] = 0;
            }
            return arr;
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

            /*
            DiscreteArray(std::vector<T *> &data, std::vector<u32> &lengths)
                    : data(data) {
                assert(data.size() == lengths.size());

                range.emplace_back(0);
                copy(begin(lengths), end(lengths), back_inserter(range));
                u32 sum = 0;
                for (auto &r: range) {
                    sum += r;
                    r = sum;
                }
            }
             */

            DiscreteArray(std::vector<T *> &data, std::vector<u32> &range)
                    : data(data), range(range) {}

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

            void add(std::vector<T> &chunk) {
                debug::print("called &"); // TODO
                data.emplace_back(chunk.data());
                range.emplace_back(range.back() + chunk.size());
            }

            void add(std::vector<T> &&chunk) {
                data.emplace_back(chunk.data());
                range.emplace_back(range.back() + chunk.size());
            }

            void mod_last(u32 length) {
                const auto size = range.size();
                if (size == 1) return; // no contents

                range[size - 1] = range[size - 2] + length;
            }

            // significantly slower than normal index access on a single array
            [[deprecated("Recommended to call with hint")]]
            T &operator()(u32 index) {
                const auto n = std::distance(begin(range) + 1,
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }

            // TODO
            // faster than normal index access on a single array
            T operator()(u32 index, u32 n, u32 dummy) const {
                // if constexpr (support_numa) data[n][index - range[n]] else data[0][index];
                return data[n][index - range[n]];
            }

            // TODO
            // faster than normal index access on a single array
            T &operator()(u32 index, u32 n) {
                // if constexpr (support_numa) data[n][index - range[n]] else data[0][index];
                return data[n][index - range[n]];
            }

            // TODO
            // significantly faster than normal index access on a single array
            /*
            T &operator()(u32 index, u32 n, u32 dummy) {
                return data[n][local_index];
            }
             */
        };
    }

    namespace parallel {
        static constexpr bool support_numa = true;
        static const u32 num_threads = std::thread::hardware_concurrency();
        static const u32 num_numa_nodes = 2;
    }

    namespace mock {
        template<class T>
        [[deprecated("Mock")]]
        static inline T *numa_alloc_onnode(size_t size, int node) {
            // TODO: constexpr (pended by CLion)
            if (parallel::support_numa) {
                // TODO
                return reinterpret_cast<T *>(malloc(size));
            } else {
                return reinterpret_cast<T *>(malloc(size));
            }
        }

        [[deprecated("Mock")]]
        static inline u32 thread_to_numa(u32 thread_id) {
            return thread_id < 2 ? 0 : 1;
        }
    }

    namespace parallel {
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
            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                const auto numa_id = mock::thread_to_numa(thread_id);
                f(numa_id, thread_id, boundaries[thread_id], boundaries[thread_id + 1]);
            }
        }

        template<class Func>
        static inline void each_numa_node(const u32 *const boundaries, Func f) {
            u32 numa_id = 0;
            u32 lower = boundaries[0];

            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                if (thread_id == num_threads - 1 || numa_id != mock::thread_to_numa(thread_id + 1)) {
                    f(numa_id, lower, boundaries[thread_id + 1]);

                    numa_id++; // TODO: mock::thread_to_numa
                    lower = boundaries[thread_id + 1];
                }
            }
        }

        template<class Func>
        static inline void each_numa_node(const u32 *const boundaries,
                                          const heap::DiscreteArray<u32> &offsets,
                                          Func f) {
            u32 numa_id = 0;
            u32 lower = boundaries[0];

            for (u32 thread_id = 0; thread_id < num_threads; ++thread_id) {
                if (thread_id == num_threads - 1 || numa_id != mock::thread_to_numa(thread_id + 1)) {
                    const auto start = offsets(lower, numa_id, 0);
                    const auto end = numa_id != num_numa_nodes - 1
                                     ? offsets(boundaries[thread_id + 1], numa_id + 1, 0)
                                     : offsets(boundaries[thread_id + 1], numa_id, 0);

                    f(numa_id, start, end);

                    numa_id++; // TODO: mock::thread_to_numa
                    lower = boundaries[thread_id + 1];
                }
            }
        }
    }
}

#endif //HOSHIZORA_INCLUDES_H
