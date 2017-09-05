#ifndef HOSHIZORA_INCLUDES_H
#define HOSHIZORA_INCLUDES_H

#include <cstdint>
#include <stdlib.h>
#include <vector>
#include <cassert>
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

        template<class T>
        struct DiscreteArray {
            std::vector<T *> data;
            std::vector<u32> range;

            DiscreteArray() = default;

            DiscreteArray(std::vector<T *> &data, std::vector<u32> &lengths)
                    : data(data), range(lengths) {
                assert(data.size() == lengths.size());

                u32 sum = 0;
                for (auto &r: range) {
                    sum += r;
                    r = sum;
                }
            }

            /*
            const T &operator[](u32 index) const {
                const auto n = std::distance(begin(range),
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }*/


            T &operator[](u32 index) &{
                const auto n = std::distance(begin(range),
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }

            /*
            T operator[](u32 index) &&{
                const auto n = std::distance(begin(range),
                                             upper_bound(begin(range), end(range), index));
                return data[n][index - range[n]];
            }*/

            /*
            T &operator[](u32 index, u32 n) {
                return data[n][index - range[n]];
            }
             */

            void add(u32 node, T *datum, u32 length) {
                nodes.emplace_back(node);
                data.emplace_back(datum);
                range.emplace_back(range.back() + length);
            }

            T get(u32 index, u32 hint) {
                return data[hint][index - range[hint]];
            }
        };
    }

    namespace mock {
        template<class T>
        static inline T *numa_alloc_onnode(size_t size, int node) {
            return nullptr;
        }
    }

    namespace debug {
        template<class T>
        static inline void print(T const &el) {
            std::cout << el << std::endl;
        }
    }
}

#endif //HOSHIZORA_INCLUDES_H
