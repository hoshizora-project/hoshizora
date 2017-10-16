#ifndef SINGLE_DECODE
#define SINGLE_DECODE

#include <iostream>
#include <stdlib.h>
#include <string>
#include <functional>
#include <type_traits>
#include <immintrin.h>

#include "common.h"

using namespace std;

template<
  uint8_t f0,
  uint8_t f1,
  uint8_t f2,
  uint8_t f3,
  uint8_t f4,
  uint8_t f5,
  uint8_t f6>
  static inline uint8_t ungap(const uint8_t *__restrict const in, const uint32_t prev, uint32_t *__restrict const out) {
  constexpr auto o0 = 0;
  constexpr auto o1 = o0 + (f0 ? 4 : 2);
  constexpr auto o2 = o1 + (f1 ? 4 : 2);
  constexpr auto o3 = o2 + (f2 ? 4 : 2);
  constexpr auto o4 = o3 + (f3 ? 4 : 2);
  constexpr auto o5 = o4 + (f4 ? 4 : 2);
  constexpr auto o6 = o5 + (f5 ? 4 : 2);
  constexpr auto total = o6 + (f6 ? 4 : 2);

  if constexpr (f0) {
    out[0] = prev + reinterpret_cast<const uint32_t *__restrict const>(in)[0];
  }
  else {
    out[0] = prev + reinterpret_cast<const uint16_t *__restrict const>(in)[0];
  }

  if constexpr (f1) {
    out[1] = out[0] + reinterpret_cast<const uint32_t *__restrict const>(in + o1)[0];
  }
  else {
    out[1] = out[0] + reinterpret_cast<const uint16_t *__restrict const>(in + o1)[0];
  }

  if constexpr (f2) {
    out[2] = out[1] + reinterpret_cast<const uint32_t *__restrict const>(in + o2)[0];
  }
  else {
    out[2] = out[1] + reinterpret_cast<const uint16_t *__restrict const>(in + o2)[0];
  }

  if constexpr (f3) {
    out[3] = out[2] + reinterpret_cast<const uint32_t *__restrict const>(in + o3)[0];
  }
  else {
    out[3] = out[2] + reinterpret_cast<const uint16_t *__restrict const>(in + o3)[0];
  }

  if constexpr (f4) {
    out[4] = out[3] + reinterpret_cast<const uint32_t *__restrict const>(in + o4)[0];
  }
  else {
    out[4] = out[3] + reinterpret_cast<const uint16_t *__restrict const>(in + o4)[0];
  }

  if constexpr (f5) {
    out[5] = out[4] + reinterpret_cast<const uint32_t *__restrict const>(in + o5)[0];
  }
  else {
    out[5] = out[4] + reinterpret_cast<const uint16_t *__restrict const>(in + o5)[0];
  }

  if constexpr (f6) {
    out[6] = out[5] + reinterpret_cast<const uint32_t *__restrict const>(in + o6)[0];
  }
  else {
    out[6] = out[5] + reinterpret_cast<const uint16_t *__restrict const>(in + o6)[0];
  }

  return total;
}

const static function<uint8_t(const uint8_t *__restrict const, const uint32_t, uint32_t *__restrict const)> ungaps[256] = {
    ungap<0, 0, 0, 0, 0, 0, 0>,
    ungap<1, 0, 0, 0, 0, 0, 0>,
    ungap<0, 1, 0, 0, 0, 0, 0>,
    ungap<1, 1, 0, 0, 0, 0, 0>,
    ungap<0, 0, 1, 0, 0, 0, 0>,
    ungap<1, 0, 1, 0, 0, 0, 0>,
    ungap<0, 1, 1, 0, 0, 0, 0>,
    ungap<1, 1, 1, 0, 0, 0, 0>,
    ungap<0, 0, 0, 1, 0, 0, 0>,
    ungap<1, 0, 0, 1, 0, 0, 0>,
    ungap<0, 1, 0, 1, 0, 0, 0>,
    ungap<1, 1, 0, 1, 0, 0, 0>,
    ungap<0, 0, 1, 1, 0, 0, 0>,
    ungap<1, 0, 1, 1, 0, 0, 0>,
    ungap<0, 1, 1, 1, 0, 0, 0>,
    ungap<1, 1, 1, 1, 0, 0, 0>,
    ungap<0, 0, 0, 0, 1, 0, 0>,
    ungap<1, 0, 0, 0, 1, 0, 0>,
    ungap<0, 1, 0, 0, 1, 0, 0>,
    ungap<1, 1, 0, 0, 1, 0, 0>,
    ungap<0, 0, 1, 0, 1, 0, 0>,
    ungap<1, 0, 1, 0, 1, 0, 0>,
    ungap<0, 1, 1, 0, 1, 0, 0>,
    ungap<1, 1, 1, 0, 1, 0, 0>,
    ungap<0, 0, 0, 1, 1, 0, 0>,
    ungap<1, 0, 0, 1, 1, 0, 0>,
    ungap<0, 1, 0, 1, 1, 0, 0>,
    ungap<1, 1, 0, 1, 1, 0, 0>,
    ungap<0, 0, 1, 1, 1, 0, 0>,
    ungap<1, 0, 1, 1, 1, 0, 0>,
    ungap<0, 1, 1, 1, 1, 0, 0>,
    ungap<1, 1, 1, 1, 1, 0, 0>,
    ungap<0, 0, 0, 0, 0, 1, 0>,
    ungap<1, 0, 0, 0, 0, 1, 0>,
    ungap<0, 1, 0, 0, 0, 1, 0>,
    ungap<1, 1, 0, 0, 0, 1, 0>,
    ungap<0, 0, 1, 0, 0, 1, 0>,
    ungap<1, 0, 1, 0, 0, 1, 0>,
    ungap<0, 1, 1, 0, 0, 1, 0>,
    ungap<1, 1, 1, 0, 0, 1, 0>,
    ungap<0, 0, 0, 1, 0, 1, 0>,
    ungap<1, 0, 0, 1, 0, 1, 0>,
    ungap<0, 1, 0, 1, 0, 1, 0>,
    ungap<1, 1, 0, 1, 0, 1, 0>,
    ungap<0, 0, 1, 1, 0, 1, 0>,
    ungap<1, 0, 1, 1, 0, 1, 0>,
    ungap<0, 1, 1, 1, 0, 1, 0>,
    ungap<1, 1, 1, 1, 0, 1, 0>,
    ungap<0, 0, 0, 0, 1, 1, 0>,
    ungap<1, 0, 0, 0, 1, 1, 0>,
    ungap<0, 1, 0, 0, 1, 1, 0>,
    ungap<1, 1, 0, 0, 1, 1, 0>,
    ungap<0, 0, 1, 0, 1, 1, 0>,
    ungap<1, 0, 1, 0, 1, 1, 0>,
    ungap<0, 1, 1, 0, 1, 1, 0>,
    ungap<1, 1, 1, 0, 1, 1, 0>,
    ungap<0, 0, 0, 1, 1, 1, 0>,
    ungap<1, 0, 0, 1, 1, 1, 0>,
    ungap<0, 1, 0, 1, 1, 1, 0>,
    ungap<1, 1, 0, 1, 1, 1, 0>,
    ungap<0, 0, 1, 1, 1, 1, 0>,
    ungap<1, 0, 1, 1, 1, 1, 0>,
    ungap<0, 1, 1, 1, 1, 1, 0>,
    ungap<1, 1, 1, 1, 1, 1, 0>,
    ungap<0, 0, 0, 0, 0, 0, 1>,
    ungap<1, 0, 0, 0, 0, 0, 1>,
    ungap<0, 1, 0, 0, 0, 0, 1>,
    ungap<1, 1, 0, 0, 0, 0, 1>,
    ungap<0, 0, 1, 0, 0, 0, 1>,
    ungap<1, 0, 1, 0, 0, 0, 1>,
    ungap<0, 1, 1, 0, 0, 0, 1>,
    ungap<1, 1, 1, 0, 0, 0, 1>,
    ungap<0, 0, 0, 1, 0, 0, 1>,
    ungap<1, 0, 0, 1, 0, 0, 1>,
    ungap<0, 1, 0, 1, 0, 0, 1>,
    ungap<1, 1, 0, 1, 0, 0, 1>,
    ungap<0, 0, 1, 1, 0, 0, 1>,
    ungap<1, 0, 1, 1, 0, 0, 1>,
    ungap<0, 1, 1, 1, 0, 0, 1>,
    ungap<1, 1, 1, 1, 0, 0, 1>,
    ungap<0, 0, 0, 0, 1, 0, 1>,
    ungap<1, 0, 0, 0, 1, 0, 1>,
    ungap<0, 1, 0, 0, 1, 0, 1>,
    ungap<1, 1, 0, 0, 1, 0, 1>,
    ungap<0, 0, 1, 0, 1, 0, 1>,
    ungap<1, 0, 1, 0, 1, 0, 1>,
    ungap<0, 1, 1, 0, 1, 0, 1>,
    ungap<1, 1, 1, 0, 1, 0, 1>,
    ungap<0, 0, 0, 1, 1, 0, 1>,
    ungap<1, 0, 0, 1, 1, 0, 1>,
    ungap<0, 1, 0, 1, 1, 0, 1>,
    ungap<1, 1, 0, 1, 1, 0, 1>,
    ungap<0, 0, 1, 1, 1, 0, 1>,
    ungap<1, 0, 1, 1, 1, 0, 1>,
    ungap<0, 1, 1, 1, 1, 0, 1>,
    ungap<1, 1, 1, 1, 1, 0, 1>,
    ungap<0, 0, 0, 0, 0, 1, 1>,
    ungap<1, 0, 0, 0, 0, 1, 1>,
    ungap<0, 1, 0, 0, 0, 1, 1>,
    ungap<1, 1, 0, 0, 0, 1, 1>,
    ungap<0, 0, 1, 0, 0, 1, 1>,
    ungap<1, 0, 1, 0, 0, 1, 1>,
    ungap<0, 1, 1, 0, 0, 1, 1>,
    ungap<1, 1, 1, 0, 0, 1, 1>,
    ungap<0, 0, 0, 1, 0, 1, 1>,
    ungap<1, 0, 0, 1, 0, 1, 1>,
    ungap<0, 1, 0, 1, 0, 1, 1>,
    ungap<1, 1, 0, 1, 0, 1, 1>,
    ungap<0, 0, 1, 1, 0, 1, 1>,
    ungap<1, 0, 1, 1, 0, 1, 1>,
    ungap<0, 1, 1, 1, 0, 1, 1>,
    ungap<1, 1, 1, 1, 0, 1, 1>,
    ungap<0, 0, 0, 0, 1, 1, 1>,
    ungap<1, 0, 0, 0, 1, 1, 1>,
    ungap<0, 1, 0, 0, 1, 1, 1>,
    ungap<1, 1, 0, 0, 1, 1, 1>,
    ungap<0, 0, 1, 0, 1, 1, 1>,
    ungap<1, 0, 1, 0, 1, 1, 1>,
    ungap<0, 1, 1, 0, 1, 1, 1>,
    ungap<1, 1, 1, 0, 1, 1, 1>,
    ungap<0, 0, 0, 1, 1, 1, 1>,
    ungap<1, 0, 0, 1, 1, 1, 1>,
    ungap<0, 1, 0, 1, 1, 1, 1>,
    ungap<1, 1, 0, 1, 1, 1, 1>,
    ungap<0, 0, 1, 1, 1, 1, 1>,
    ungap<1, 0, 1, 1, 1, 1, 1>,
    ungap<0, 1, 1, 1, 1, 1, 1>,
    ungap<1, 1, 1, 1, 1, 1, 1>,
    ungap<0, 0, 0, 0, 0, 0, 0>,
    ungap<1, 0, 0, 0, 0, 0, 0>,
    ungap<0, 1, 0, 0, 0, 0, 0>,
    ungap<1, 1, 0, 0, 0, 0, 0>,
    ungap<0, 0, 1, 0, 0, 0, 0>,
    ungap<1, 0, 1, 0, 0, 0, 0>,
    ungap<0, 1, 1, 0, 0, 0, 0>,
    ungap<1, 1, 1, 0, 0, 0, 0>,
    ungap<0, 0, 0, 1, 0, 0, 0>,
    ungap<1, 0, 0, 1, 0, 0, 0>,
    ungap<0, 1, 0, 1, 0, 0, 0>,
    ungap<1, 1, 0, 1, 0, 0, 0>,
    ungap<0, 0, 1, 1, 0, 0, 0>,
    ungap<1, 0, 1, 1, 0, 0, 0>,
    ungap<0, 1, 1, 1, 0, 0, 0>,
    ungap<1, 1, 1, 1, 0, 0, 0>,
    ungap<0, 0, 0, 0, 1, 0, 0>,
    ungap<1, 0, 0, 0, 1, 0, 0>,
    ungap<0, 1, 0, 0, 1, 0, 0>,
    ungap<1, 1, 0, 0, 1, 0, 0>,
    ungap<0, 0, 1, 0, 1, 0, 0>,
    ungap<1, 0, 1, 0, 1, 0, 0>,
    ungap<0, 1, 1, 0, 1, 0, 0>,
    ungap<1, 1, 1, 0, 1, 0, 0>,
    ungap<0, 0, 0, 1, 1, 0, 0>,
    ungap<1, 0, 0, 1, 1, 0, 0>,
    ungap<0, 1, 0, 1, 1, 0, 0>,
    ungap<1, 1, 0, 1, 1, 0, 0>,
    ungap<0, 0, 1, 1, 1, 0, 0>,
    ungap<1, 0, 1, 1, 1, 0, 0>,
    ungap<0, 1, 1, 1, 1, 0, 0>,
    ungap<1, 1, 1, 1, 1, 0, 0>,
    ungap<0, 0, 0, 0, 0, 1, 0>,
    ungap<1, 0, 0, 0, 0, 1, 0>,
    ungap<0, 1, 0, 0, 0, 1, 0>,
    ungap<1, 1, 0, 0, 0, 1, 0>,
    ungap<0, 0, 1, 0, 0, 1, 0>,
    ungap<1, 0, 1, 0, 0, 1, 0>,
    ungap<0, 1, 1, 0, 0, 1, 0>,
    ungap<1, 1, 1, 0, 0, 1, 0>,
    ungap<0, 0, 0, 1, 0, 1, 0>,
    ungap<1, 0, 0, 1, 0, 1, 0>,
    ungap<0, 1, 0, 1, 0, 1, 0>,
    ungap<1, 1, 0, 1, 0, 1, 0>,
    ungap<0, 0, 1, 1, 0, 1, 0>,
    ungap<1, 0, 1, 1, 0, 1, 0>,
    ungap<0, 1, 1, 1, 0, 1, 0>,
    ungap<1, 1, 1, 1, 0, 1, 0>,
    ungap<0, 0, 0, 0, 1, 1, 0>,
    ungap<1, 0, 0, 0, 1, 1, 0>,
    ungap<0, 1, 0, 0, 1, 1, 0>,
    ungap<1, 1, 0, 0, 1, 1, 0>,
    ungap<0, 0, 1, 0, 1, 1, 0>,
    ungap<1, 0, 1, 0, 1, 1, 0>,
    ungap<0, 1, 1, 0, 1, 1, 0>,
    ungap<1, 1, 1, 0, 1, 1, 0>,
    ungap<0, 0, 0, 1, 1, 1, 0>,
    ungap<1, 0, 0, 1, 1, 1, 0>,
    ungap<0, 1, 0, 1, 1, 1, 0>,
    ungap<1, 1, 0, 1, 1, 1, 0>,
    ungap<0, 0, 1, 1, 1, 1, 0>,
    ungap<1, 0, 1, 1, 1, 1, 0>,
    ungap<0, 1, 1, 1, 1, 1, 0>,
    ungap<1, 1, 1, 1, 1, 1, 0>,
    ungap<0, 0, 0, 0, 0, 0, 1>,
    ungap<1, 0, 0, 0, 0, 0, 1>,
    ungap<0, 1, 0, 0, 0, 0, 1>,
    ungap<1, 1, 0, 0, 0, 0, 1>,
    ungap<0, 0, 1, 0, 0, 0, 1>,
    ungap<1, 0, 1, 0, 0, 0, 1>,
    ungap<0, 1, 1, 0, 0, 0, 1>,
    ungap<1, 1, 1, 0, 0, 0, 1>,
    ungap<0, 0, 0, 1, 0, 0, 1>,
    ungap<1, 0, 0, 1, 0, 0, 1>,
    ungap<0, 1, 0, 1, 0, 0, 1>,
    ungap<1, 1, 0, 1, 0, 0, 1>,
    ungap<0, 0, 1, 1, 0, 0, 1>,
    ungap<1, 0, 1, 1, 0, 0, 1>,
    ungap<0, 1, 1, 1, 0, 0, 1>,
    ungap<1, 1, 1, 1, 0, 0, 1>,
    ungap<0, 0, 0, 0, 1, 0, 1>,
    ungap<1, 0, 0, 0, 1, 0, 1>,
    ungap<0, 1, 0, 0, 1, 0, 1>,
    ungap<1, 1, 0, 0, 1, 0, 1>,
    ungap<0, 0, 1, 0, 1, 0, 1>,
    ungap<1, 0, 1, 0, 1, 0, 1>,
    ungap<0, 1, 1, 0, 1, 0, 1>,
    ungap<1, 1, 1, 0, 1, 0, 1>,
    ungap<0, 0, 0, 1, 1, 0, 1>,
    ungap<1, 0, 0, 1, 1, 0, 1>,
    ungap<0, 1, 0, 1, 1, 0, 1>,
    ungap<1, 1, 0, 1, 1, 0, 1>,
    ungap<0, 0, 1, 1, 1, 0, 1>,
    ungap<1, 0, 1, 1, 1, 0, 1>,
    ungap<0, 1, 1, 1, 1, 0, 1>,
    ungap<1, 1, 1, 1, 1, 0, 1>,
    ungap<0, 0, 0, 0, 0, 1, 1>,
    ungap<1, 0, 0, 0, 0, 1, 1>,
    ungap<0, 1, 0, 0, 0, 1, 1>,
    ungap<1, 1, 0, 0, 0, 1, 1>,
    ungap<0, 0, 1, 0, 0, 1, 1>,
    ungap<1, 0, 1, 0, 0, 1, 1>,
    ungap<0, 1, 1, 0, 0, 1, 1>,
    ungap<1, 1, 1, 0, 0, 1, 1>,
    ungap<0, 0, 0, 1, 0, 1, 1>,
    ungap<1, 0, 0, 1, 0, 1, 1>,
    ungap<0, 1, 0, 1, 0, 1, 1>,
    ungap<1, 1, 0, 1, 0, 1, 1>,
    ungap<0, 0, 1, 1, 0, 1, 1>,
    ungap<1, 0, 1, 1, 0, 1, 1>,
    ungap<0, 1, 1, 1, 0, 1, 1>,
    ungap<1, 1, 1, 1, 0, 1, 1>,
    ungap<0, 0, 0, 0, 1, 1, 1>,
    ungap<1, 0, 0, 0, 1, 1, 1>,
    ungap<0, 1, 0, 0, 1, 1, 1>,
    ungap<1, 1, 0, 0, 1, 1, 1>,
    ungap<0, 0, 1, 0, 1, 1, 1>,
    ungap<1, 0, 1, 0, 1, 1, 1>,
    ungap<0, 1, 1, 0, 1, 1, 1>,
    ungap<1, 1, 1, 0, 1, 1, 1>,
    ungap<0, 0, 0, 1, 1, 1, 1>,
    ungap<1, 0, 0, 1, 1, 1, 1>,
    ungap<0, 1, 0, 1, 1, 1, 1>,
    ungap<1, 1, 0, 1, 1, 1, 1>,
    ungap<0, 0, 1, 1, 1, 1, 1>,
    ungap<1, 0, 1, 1, 1, 1, 1>,
    ungap<0, 1, 1, 1, 1, 1, 1>,
    ungap<1, 1, 1, 1, 1, 1, 1>
};

static uint32_t decode(const uint8_t *__restrict const in, const uint32_t size, uint32_t *__restrict const out) {
  if (size > 0u) {
    auto out_offset = 0ul;
    auto in_offset = 0ul;
    const auto n_blocks = size / LENGTH;

    if (n_blocks) {
      const auto n_flag_blocks = (n_blocks + 1u) / 2u;
      const uint32_t n_flag_blocks_align32 = ((n_flag_blocks + 31u) / 32u) * 32u;
      in_offset += n_flag_blocks_align32;

      a32_vector<uint8_t> flags(n_blocks + 1u);
      const auto N = n_flag_blocks / YMM_BYTE;
      for (auto i = 0ul; i < N; i++) {
        const auto reg = _mm256_load_si256(reinterpret_cast<__m256icpc>(in + YMM_BYTE * i));
        _mm256_store_si256(reinterpret_cast<__m256ipc>(flags.data() + YMM_BYTE * (i * 2u)),
          _mm256_and_si256(reg,
            _mm256_load_si256(reinterpret_cast<__m256icpc>(mask8r[4]))));
        _mm256_store_si256(reinterpret_cast<__m256ipc>(flags.data() + YMM_BYTE * (i * 2u + 1u)),
          _mm256_srli_epi8(reg, 4u));
      }
      for (auto i = N * YMM_BYTE * 2u; i < n_blocks; i += 2u) {
        flags[i] = in[i / 2u] & 0xFu;
        flags[i + 1u] = in[i / 2u] >> 4u;
      }

      uint8_t n_used_bits = 0u;
      auto prev = _mm256_setzero_si256();
      auto reg = _mm256_load_si256(reinterpret_cast<__m256icpc>(in + in_offset));
      for (auto i = 0ul; i < n_blocks; i++) {
        const uint32_t pack_size = pack_sizes[flags[i]];

        if (n_used_bits + pack_size > BIT_PER_BOX) {
          // mv next
          in_offset += YMM_BYTE;
          reg = _mm256_load_si256(reinterpret_cast<__m256icpc>(in + in_offset));
          const auto for_store = _mm256_add_epi32(prev,
            _mm256_and_si256(
              reg,
              _mm256_load_si256(
                reinterpret_cast<__m256icpc>(mask32r[pack_size]))));
          _mm256_stream_si256(reinterpret_cast<__m256ipc>(out + out_offset), for_store);
          prev = _mm256_permutevar8x32_epi32(for_store, broadcast_mask);
          n_used_bits = pack_size;
        }
        else {
          // continue using curr
          const auto for_store = _mm256_add_epi32(prev,
            _mm256_and_si256(
              _mm256_srli_epi32(reg, n_used_bits),
              _mm256_load_si256(
                reinterpret_cast<__m256icpc>(mask32r[pack_size]))));
          _mm256_stream_si256(reinterpret_cast<__m256ipc>(out + out_offset), for_store);
          prev = _mm256_permutevar8x32_epi32(for_store, broadcast_mask);
          n_used_bits += pack_size;
        }

        out_offset += LENGTH;
      }

      if (n_used_bits > 0u) {
        in_offset += YMM_BYTE;
      }
    }

    const uint16_t remain = size - out_offset;
    if (remain > 0u) {
      const auto prev = out_offset == 0 ? 0 : out[out_offset - 1u];
      const auto consumed = ungaps[in[in_offset]](
        in + in_offset + 1u, prev, out + out_offset);

      in_offset += 1u + consumed;
      out_offset += LENGTH - 1u;

      // revert overrun
      in_offset -= (out_offset - size) * 2u;
      /*out_offset -= out_offset - size;*/
    }

    return in_offset;
  }
  else {
    return 0ul;
  }
}

template<typename Func>
static uint32_t traverse(const uint8_t *__restrict const in, const uint32_t size, Func f) {
  if (size > 0u) {
    auto out_offset = 0ul;
    auto in_offset = 0ul;
    const auto n_blocks = size / LENGTH;

    alignas(32) uint32_t out[LENGTH];

    if (n_blocks) {
      const auto n_flag_blocks = (n_blocks + 1u) / 2u;
      const uint32_t n_flag_blocks_align32 = ((n_flag_blocks + 31u) / 32u) * 32u;
      in_offset += n_flag_blocks_align32;

      a32_vector<uint8_t> flags(n_blocks + 1u);
      const auto N = n_flag_blocks / YMM_BYTE;
      for (auto i = 0ul; i < N; i++) {
        const auto reg = _mm256_load_si256(reinterpret_cast<__m256icpc>(in + YMM_BYTE * i));
        _mm256_store_si256(reinterpret_cast<__m256ipc>(flags.data() + YMM_BYTE * (i * 2u)),
          _mm256_and_si256(reg,
            _mm256_load_si256(reinterpret_cast<__m256icpc>(mask8r[4]))));
        _mm256_store_si256(reinterpret_cast<__m256ipc>(flags.data() + YMM_BYTE * (i * 2u + 1u)),
          _mm256_srli_epi8(reg, 4u));
      }
      for (auto i = N * YMM_BYTE * 2u; i < n_blocks; i += 2u) {
        flags[i] = in[i / 2u] & 0xFu;
        flags[i + 1u] = in[i / 2u] >> 4u;
      }

      uint8_t n_used_bits = 0u;
      auto prev = _mm256_setzero_si256();
      auto reg = _mm256_load_si256(reinterpret_cast<__m256icpc>(in + in_offset));
      for (auto i = 0ul; i < n_blocks; i++) {
        const uint32_t pack_size = pack_sizes[flags[i]];

        if (n_used_bits + pack_size > BIT_PER_BOX) {
          // mv next
          in_offset += YMM_BYTE;
          reg = _mm256_load_si256(reinterpret_cast<__m256icpc>(in + in_offset));
          const auto for_store = _mm256_add_epi32(prev,
            _mm256_and_si256(
              reg,
              _mm256_load_si256(
                reinterpret_cast<__m256icpc>(mask32r[pack_size]))));
          _mm256_store_si256(reinterpret_cast<__m256ipc>(out), for_store);
          for (auto j = 0ul; j < LENGTH; j++) {
            f(out[j], out_offset + j);
          }
          prev = _mm256_permutevar8x32_epi32(for_store, broadcast_mask);
          n_used_bits = pack_size;
        }
        else {
          // continue using curr
          const auto for_store = _mm256_add_epi32(prev,
            _mm256_and_si256(
              _mm256_srli_epi32(reg, n_used_bits),
              _mm256_load_si256(
                reinterpret_cast<__m256icpc>(mask32r[pack_size]))));
          _mm256_store_si256(reinterpret_cast<__m256ipc>(out), for_store);
          for (auto j = 0ul; j < LENGTH; j++) {
            f(out[j], out_offset + j);
          }
          prev = _mm256_permutevar8x32_epi32(for_store, broadcast_mask);
          n_used_bits += pack_size;
        }

        out_offset += LENGTH;
      }

      if (n_used_bits > 0u) {
        in_offset += YMM_BYTE;
      }
    }

    const uint16_t remain = size - out_offset;
    if (remain > 0u) {
      const auto consumed = ungaps[in[in_offset]](
        in + in_offset + 1u, out[LENGTH - 1u], out);
      for (auto i = 0u; i < remain; i++) {
        f(out[i], out_offset + i);
      }

      in_offset += 1u + consumed;
      out_offset += LENGTH - 1u;

      // revert overrun
      in_offset -= (out_offset - size) * 2u;
      /*out_offset -= out_offset - size;*/
    }

    return in_offset;
  }
  else {
    return 0ul;
  }
}

#endif // SINGLE_DECODE
