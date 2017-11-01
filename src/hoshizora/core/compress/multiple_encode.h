#ifndef MULTIPLE_ENCODE_H
#define MULTIPLE_ENCODE_H

#include <iostream>
#include <chrono>
#include <string>
#include <immintrin.h>

#include "hoshizora/core/util/includes.h"
#include "hoshizora/core/compress/common.h"
#include "hoshizora/core/compress/single_encode.h"

namespace hoshizora::compress {
    // |offsets| should be n_lists + 1
    static u32 multiple_encode(const u32 *__restrict const in,
                               const u32 *__restrict const offsets, const u32 n_lists,
                               u8 *__restrict const out) {
        u32 out_consumed = 0;
        out_consumed += single_encode(offsets, n_lists + 1u, out);
        out_consumed = ((out_consumed + 31u) / 32u) * 32u;

        u32 i = 0;
        while (true) {
            auto head = in + offsets[i];
            u32 len = offsets[i + 1] - offsets[i];
            if (len > THRESHOLD) {
                out_consumed += single_encode(head, len, out + out_consumed);
                i++;
            } else {
                u32 len_acc = 0;
                while (true) {
                    if ((len >= THRESHOLD && len_acc >= 256u) || i == n_lists) {
                        break;
                    }
                    len = offsets[i + 1] - offsets[i];
                    len_acc += len;
                    i++;
                }
                {
                    size_t consumed = len_acc;
                    pfor->encodeArray(head, len_acc,
                                      reinterpret_cast<u32 *__restrict const>(out + out_consumed), consumed);
                    out_consumed += consumed * 4u;
                }
            }

            if (i == n_lists) {
                break;
            } else {
                out_consumed = ((out_consumed + 31u) / 32u) * 32u;
            }
        }
        return out_consumed;
    }
}
#endif // MULTIPLE_ENCODE_H
