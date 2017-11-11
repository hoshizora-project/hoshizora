#ifndef MULTIPLE_H
#define MULTIPLE_H

#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <string>

#include "hoshizora/core/compress/common.h"
#include "hoshizora/core/compress/single.h"
#include "hoshizora/core/util/includes.h"

namespace hoshizora::compress::multiple {
/*
 * encode
 */
// |offsets| should be n_lists + 1
static u32 encode(const u32 *__restrict const in,
                  const u32 *__restrict const offsets,
                  const u32 num_inner_lists, u8 *__restrict const out) {
  u32 out_consumed = single::encode(offsets, num_inner_lists + 1u, out);

  u32 i = 0;
  while (true) {
    auto head = in + offsets[i];
    u32 len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      // TODO
      auto _head = head;
      if ((reinterpret_cast<uintptr_t>(_head) & 15) != 0) {
        const auto tmp = new a32_vector<u32>(head, head + len);
        _head = tmp->data();
      }
      out_consumed += single::encode(_head, len, out + out_consumed);
      i++;
    } else {
      u32 len_acc = 0;
      while (true) {
        if ((len >= THRESHOLD && len_acc >= 256u) || i == num_inner_lists) {
          break;
        }
        len = offsets[i + 1] - offsets[i];
        len_acc += len;
        i++;
      }
      {
        // FIXME: consumed represents #remainBlocks of out
        size_t consumed = len_acc;
        // TODO
        auto _head = head;
        if ((reinterpret_cast<uintptr_t>(_head) & 15) != 0) {
          const auto tmp = new a32_vector<u32>(head, head + len_acc);
          _head = tmp->data();
        }
        pfor->encodeArray(_head, len_acc,
                          reinterpret_cast<u32 *const>(out + out_consumed),
                          consumed);
        out_consumed += consumed * 4u;
      }
    }

    out_consumed = ((out_consumed + 31u) / 32u) * 32u;
    if (i == num_inner_lists) {
      break;
    }
  }

  return out_consumed;
}

// |offsets| should be n_lists + 1
static u32 estimate(const u32 *__restrict const in,
                    const u32 *__restrict const offsets,
                    const u32 num_inner_lists) {
  u32 out_consumed = single::estimate(offsets, num_inner_lists + 1u);

  u32 i = 0;
  while (true) {
    auto head = in + offsets[i];
    u32 len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      out_consumed += single::estimate(head, len);
      i++;
    } else {
      u32 len_acc = 0;
      while (true) {
        if ((len >= THRESHOLD && len_acc >= 256u) || i == num_inner_lists) {
          break;
        }
        len = offsets[i + 1] - offsets[i];
        len_acc += len;
        i++;
      }
      { out_consumed += pfor->estimate(head, len_acc) * 4u; }
    }

    out_consumed = ((out_consumed + 31u) / 32u) * 32u;
    if (i == num_inner_lists) {
      break;
    }
  }

  return out_consumed;
}

/*
 * decode
 */
static void decode(const u8 *const __restrict in, const u32 num_inner_lists,
                   u32 *__restrict const out, u32 *__restrict const offsets) {
  alignas(32) u32 buffer[100000]; // TODO: must be unused
  u32 in_consumed = single::decode(in, num_inner_lists + 1u, offsets);
  in_consumed = ((in_consumed + 31u) / 32u) * 32u;
  u32 out_consumed = 0;

  u32 i = 0;
  while (true) {
    auto head = in + in_consumed;
    u32 len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      in_consumed += single::decode(head, len, buffer);
      std::copy(buffer, buffer + len, out + out_consumed);
      out_consumed += len;
      i++;
    } else {
      u32 len_acc = 0;
      while (true) {
        if ((len >= THRESHOLD && len_acc >= 256u) || i == num_inner_lists) {
          break;
        }
        len = offsets[i + 1] - offsets[i];
        len_acc += len;
        i++;
      }
      {
        size_t consumed = len_acc;
        const auto proceeded = reinterpret_cast<const u8 *>(pfor->decodeArray(
            reinterpret_cast<const u32 *const>(head), consumed /*dummy*/,
            buffer, consumed /*as len_acc*/));
        std::copy(buffer, buffer + len_acc, out + out_consumed);
        in_consumed += proceeded - head;
        out_consumed += len_acc;
      }
    }

    in_consumed = ((in_consumed + 31u) / 32u) * 32u;
    if (i == num_inner_lists) {
      break;
    }
  }
}

template <
    typename Func /*(unpacked_datum, local_offset, global_idx, local_idx)*/>
static void foreach (const u8 *__restrict const in, const u32 num_inner_lists,
                     Func f) {
  alignas(32) u32 buffer[100000];               // TODO: must be unused
  a32_vector<u32> offsets(num_inner_lists + 1); // TODO
  u32 in_consumed = single::decode(in, num_inner_lists + 1, offsets.data());
  in_consumed = ((in_consumed + 31u) / 32u) * 32u;

  u32 i = 0;
  while (true) {
    auto head = in + in_consumed;
    u32 len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      in_consumed +=
          single::foreach (head, len,
                           std::bind(f, std::placeholders::_1, offsets[i], i,
                                     std::placeholders::_2));
      i++;
    } else {
      u32 acc_start = i;
      u32 len_acc = 0;
      while (true) {
        if ((len >= THRESHOLD && len_acc >= 256u) || i == num_inner_lists) {
          break;
        }
        len = offsets[i + 1] - offsets[i];
        len_acc += len;
        i++;
      }
      {
        size_t consumed = len_acc;
        // FIXME
        // const auto proceeded =
        //    reinterpret_cast<const u8 *__restrict>(pfor->mapArray(
        //        reinterpret_cast<const u32 *__restrict const>(head),
        //        consumed /*dummy*/, buffer, consumed /*as len_acc*/,
        //        bind(f, std::placeholders::_1, i /*dummy*/, i /*wrong*/,
        //             std::placeholders::_2) /*temporarily ignored*/));
        const auto proceeded = reinterpret_cast<const u8 *>(pfor->decodeArray(
            reinterpret_cast<const u32 *const>(head), consumed /*dummy*/,
            buffer, consumed /*as len_acc*/));

        const auto this_offset = offsets[acc_start];
        for (u32 j = acc_start; j < i; ++j) { // src
          for (u32 start = offsets[j] - this_offset,
                   end = offsets[j + 1] - this_offset, k = start;
               k < end; ++k) {
            f(buffer[k], start, j, k - start);
          }
        }
        in_consumed += proceeded - head;
      }
    }

    in_consumed = ((in_consumed + 31u) / 32u) * 32u;
    if (i == num_inner_lists) {
      break;
    }
  }
}
} // namespace hoshizora::compress::multiple
#endif // MULTIPLE_H
