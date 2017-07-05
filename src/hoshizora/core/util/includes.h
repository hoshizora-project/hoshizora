#ifndef HOSHIZORA_INCLUDES_H
#define HOSHIZORA_INCLUDES_H

#include <cstdint>
#include <stdlib.h>
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
            for(size_t i = 0; i < length; ++i) {
                arr[i] = 0;
            }
            return arr;
        }
    }
}

#endif //HOSHIZORA_INCLUDES_H
