#ifndef HOSHIZORA_PRIMITIVE_INCLUDES_H
#define HOSHIZORA_PRIMITIVE_INCLUDES_H

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
#include <cpuid.h>
#include <mach/thread_act.h>
#endif
#ifdef SUPPORT_NUMA
#include <numa.h>
#endif
#include "external/spdlog/spdlog.h"

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
    using empty_t = std::nullptr_t[0];

    namespace debug {
        const auto logger = spdlog::stderr_color_mt("hoshizora");

        static inline void init_logger() {
            logger->set_level(spdlog::level::debug);
        }
    }

    namespace mem {
        template<class T>
        static inline T *alloc(u64 length) {
#ifdef SUPPORT_NUMA
            return static_cast<T *>(numa_alloc_local(sizeof(T) * length));
#else
            return static_cast<T *>(malloc(sizeof(T) * length));
#endif
        }

        template<class T>
        static inline T *alloc(u64 length, u32 node) {
#ifdef SUPPORT_NUMA
            return static_cast<T *>(numa_alloc_onnode(sizeof(T) * length, node));
#else
            return static_cast<T *>(malloc(sizeof(T) * length));
#endif
        }

        template<class T>
        static inline T *calloc(u64 length) {
            auto arr = alloc<T>(length);
            std::memset(arr, 0, sizeof(T) * length);
            return arr;
        }

        template<class T>
        static inline T *calloc(u64 length, u32 node) {
            auto arr = alloc<T>(length, node);
            std::memset(arr, 0, sizeof(T) * length);
            return arr;
        }

        static inline void free(void *ptr, size_t size) {
#ifdef SUPPORT_NUMA
            numa_free(ptr, size);
#else
            std::free(ptr);
#endif
        }
    }

    namespace sched {
        static inline i32 get_cpu_id() {
#ifdef __linux__
            return sched_getcpu();
#elif __APPLE__
            std::array<u32, 4> cpu_info = {{0, 0, 0, 0}};
            __cpuid_count(1, 0, cpu_info[0], cpu_info[1], cpu_info[2], cpu_info[3]);
            i32 cpu_id = -1;
            if ((cpu_info[3] & (1 << 9)) == 0) {
                /* no APIC on chip */
            } else {
                cpu_id = static_cast<u32>(cpu_info[1] >> 24);
            }
            if (cpu_id < 0) cpu_id = 0;
            return cpu_id;
#else
            return -1;
#endif
        }
    }

    namespace loop {
        static constexpr bool support_numa =
#ifdef SUPPORT_NUMA
                true;
#else
                false;
#endif
        static const u32 num_threads = std::thread::hardware_concurrency();
        static const u32 num_numa_nodes = 2;
        static const std::vector<u32> numa_boundaries = {0, 2, 4};
    }

    namespace mock {
        //[[deprecated("Mock")]]
        static inline u32 thread_to_numa(u32 thread_id) {
            return thread_id < 2 ? 0 : 1;
        }
    }
}

#endif //HOSHIZORA_PRIMITIVE_INCLUDES_H
