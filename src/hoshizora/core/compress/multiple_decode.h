#ifndef MULTIPLE_DECODE
#define MULTIPLE_DECODE

#include <iostream>
#include <stdlib.h>
#include <string>
#include <functional>
#include <type_traits>
#include <immintrin.h>

#include "common.h"
#include "external.h"
#include "single_decode.h"

using namespace std;

alignas(32) uint32_t buffer[1000000]; // TODO: must be unused

static void decode(const uint8_t *__restrict in, const uint32_t n_lists,
  uint32_t *__restrict const out, uint32_t *__restrict const offsets) {
  auto in_consumed = 0ul;
  in_consumed += decode(in, n_lists + 1u, offsets);
  in_consumed = ((in_consumed + 31u) / 32u) * 32u;

  auto out_consumed = 0ul;
  auto i = 0ul;
  while (true) {
    auto head = in + in_consumed;
    auto len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      in_consumed += decode(head, len, buffer);
      copy(buffer, buffer + len, out + out_consumed);
      out_consumed += len;
      i++;
    }
    else {
      auto len_acc = 0ul;
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
        const auto proceeded = reinterpret_cast<const uint8_t *__restrict>(pfor->decodeArray(
          reinterpret_cast<const uint32_t *__restrict const>(head),
            consumed/*dummy*/, buffer, consumed/*as len_acc*/));
        copy(buffer, buffer + len_acc, out + out_consumed);
        in_consumed += proceeded - head;
        out_consumed += len_acc;
      }
    }

    if (i == n_lists) {
      break;
    }
    else {
      in_consumed = ((in_consumed + 31u) / 32u) * 32u;
    }
  }
}

template<typename Func>
static void traverse0(const uint8_t *__restrict const in, const uint32_t n_lists,
  Func f, uint32_t *__restrict const out = nullptr,
  uint32_t *__restrict const offsets = nullptr) {
  auto in_consumed = 0ul;
  in_consumed += decode(in, n_lists + 1u, offsets);
  in_consumed = ((in_consumed + 31u) / 32u) * 32u;

  auto i = 0ul;
  while (true) {
    auto head = in + in_consumed;
    auto len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      in_consumed += traverse(head, len, bind(f, placeholders::_1, i, placeholders::_2));
      i++;
    }
    else {
      auto len_acc = 0ul;
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
        const auto proceeded = reinterpret_cast<const uint8_t *__restrict>(pfor->mapArray(
          reinterpret_cast<const uint32_t *__restrict const>(head),
          consumed/*dummy*/, buffer, consumed/*as len_acc*/,
          bind(f, placeholders::_1, i, placeholders::_2)));
        in_consumed += proceeded - head;
      }
    }

    if (i == n_lists) {
      break;
    }
    else {
      in_consumed = ((in_consumed + 31u) / 32u) * 32u;
    }
  }
}

#endif // MULTIPLE_DECODE
