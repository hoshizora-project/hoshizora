#ifndef MULTIPLE_ENCODE
#define MULTIPLE_ENCODE

#include <iostream>
#include <chrono>
#include <string>
#include <immintrin.h>

#include "common.h"
#include "external.h"
#include "single_encode.h"

using namespace std;

// |offsets| should be n_lists + 1
static uint32_t encode(const uint32_t *__restrict const in,
  const uint32_t *__restrict const offsets, const uint32_t n_lists,
  uint8_t *__restrict const out) {
  auto out_consumed = 0ul;
  out_consumed += encode(offsets, n_lists + 1u, out);
  out_consumed = ((out_consumed + 31u) / 32u) * 32u;

  auto i = 0ul;
  while (true) {
    auto head = in + offsets[i];
    auto len = offsets[i + 1] - offsets[i];
    if (len > THRESHOLD) {
      out_consumed += encode(head, len, out + out_consumed);
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
        pfor->encodeArray(head, len_acc,
          reinterpret_cast<uint32_t *__restrict const>(out + out_consumed), consumed);
        out_consumed += consumed * 4u;
      }
    }

    if (i == n_lists) {
      break;
    }
    else {
      out_consumed = ((out_consumed + 31u) / 32u) * 32u;
    }
  }
  return out_consumed;
}

#endif // MULTIPLE_ENCODE
