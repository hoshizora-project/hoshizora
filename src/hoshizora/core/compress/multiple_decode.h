#ifndef MULTIPLE_DECODE_H
#define MULTIPLE_DECODE_H

#include <functional>
#include <immintrin.h>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <type_traits>

#include "hoshizora/core/compress/common.h"
#include "hoshizora/core/compress/single_decode.h"
#include "hoshizora/core/util/includes.h"

namespace hoshizora::compress {
alignas(32) u32 buffer[1000000]; // TODO: must be unused

static void multiple_decode(const u8 *__restrict in, const u32 n_lists,
                            u32 *__restrict const out,
                            u32 *__restrict const offsets) {
  u32 in_consumed = 0;
  in_consumed += single_decode(in, n_lists + 1u, offsets);
  in_consumed = ((in_consumed + 31u) / 32u) * 32u;

  u32 out_consumed = 0;
  u32 i = 0;
  while (true) {
    auto head = in + in_consumed;
    u32 len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      in_consumed += single_decode(head, len, buffer);
      copy(buffer, buffer + len, out + out_consumed);
      out_consumed += len;
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
        const auto proceeded =
            reinterpret_cast<const u8 *__restrict>(pfor->decodeArray(
                reinterpret_cast<const u32 *__restrict const>(head),
                consumed /*dummy*/, buffer, consumed /*as len_acc*/));
        copy(buffer, buffer + len_acc, out + out_consumed);
        in_consumed += proceeded - head;
        out_consumed += len_acc;
      }
    }

    if (i == n_lists) {
      break;
    } else {
      in_consumed = ((in_consumed + 31u) / 32u) * 32u;
    }
  }
}

template <typename Func>
static void multiple_traverse(const u8 *__restrict const in, const u32 n_lists,
                              Func f, u32 *__restrict const out = nullptr,
                              u32 *__restrict const offsets = nullptr) {
  u32 in_consumed = 0;
  in_consumed += single_decode(in, n_lists + 1u, offsets);
  in_consumed = ((in_consumed + 31u) / 32u) * 32u;

  u32 i = 0;
  while (true) {
    auto head = in + in_consumed;
    u32 len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      in_consumed += single_traverse(
          head, len, bind(f, placeholders::_1, i, placeholders::_2));
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
        const auto proceeded = reinterpret_cast<const u8 *__restrict>(
            pfor->mapArray(reinterpret_cast<const u32 *__restrict const>(head),
                           consumed /*dummy*/, buffer, consumed /*as len_acc*/,
                           bind(f, placeholders::_1, i, placeholders::_2)));
        in_consumed += proceeded - head;
      }
    }

    if (i == n_lists) {
      break;
    } else {
      in_consumed = ((in_consumed + 31u) / 32u) * 32u;
    }
  }
}
} // namespace hoshizora::compress
#endif // MULTIPLE_DECODE_H
