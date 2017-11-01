#ifndef SINGLE_ENCODE_H
#define SINGLE_ENCODE_H

#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <string>

#include "hoshizora/core/compress/common.h"
#include "hoshizora/core/util/includes.h"

namespace hoshizora::compress {
static u32 single_encode(const u32 *__restrict const in, const u32 size,
                         u8 *__restrict const out) {
  if (size > 0) {
    u32 in_offset = 0;
    u32 out_offset = 0;
    const u32 n_blocks = size / 8u;
    if (n_blocks) {
      const u32 n_flag_blocks = (n_blocks + 1u) / 2u;
      const u32 n_flag_blocks_align32 = ((n_flag_blocks + 31u) / 32u) * 32u;
      out_offset += n_flag_blocks_align32;

      a32_vector<u8> flags(n_blocks + 1u);

      u8 n_used_bits = 0;
      auto prev = _mm256_setzero_si256();
      auto reg =
          _mm256_load_si256(reinterpret_cast<__m256ipc>(out + out_offset));
      alignas(32) u32 xs[LENGTH];
      for (u32 i = 0; i < n_blocks; i++) {
        const auto curr =
            _mm256_load_si256(reinterpret_cast<__m256icpc>(in + in_offset));
        const auto diff = _mm256_sub_epi32(curr, prev);

        _mm256_store_si256(reinterpret_cast<__m256ipc>(xs),
                           diff); // TODO: reg only
        const u8 pack_idx = pack_sizes_helper[_lzcnt_u32(xs[7])];
        const u32 pack_size = pack_sizes[pack_idx];
        flags[i] = pack_idx;

        if (n_used_bits + pack_size > BIT_PER_BOX) {
          // flush
          _mm256_store_si256(reinterpret_cast<__m256ipc>(out + out_offset),
                             reg);
          out_offset += YMM_BYTE;

          // mv next
          reg = diff;
          n_used_bits = static_cast<u8>(pack_size);
        } else {
          reg = _mm256_or_si256(reg, _mm256_slli_epi32(diff, n_used_bits));
          n_used_bits += pack_size;
        }

        prev = _mm256_permutevar8x32_epi32(curr, broadcast_mask);
        in_offset += LENGTH;
      }

      if (n_used_bits > 0) {
        // flush
        _mm256_store_si256(reinterpret_cast<__m256ipc>(out + out_offset), reg);
        out_offset += YMM_BYTE;
      }

      const u32 N = n_flag_blocks / YMM_BYTE;
      for (u32 i = 0; i < N; i++) {
        const auto acc = _mm256_or_si256(
            _mm256_load_si256(reinterpret_cast<__m256icpc>(
                flags.data() + YMM_BYTE * (i * 2u))),
            _mm256_slli_epi8(_mm256_load_si256(reinterpret_cast<__m256icpc>(
                                 flags.data() + YMM_BYTE * (i * 2u + 1))),
                             4u));
        _mm256_store_si256(reinterpret_cast<__m256ipc>(out + YMM_BYTE * i),
                           acc);
      }
      for (u32 i = N * YMM_BYTE * 2u; i < n_blocks; i += 2u) {
        flags[i] |= flags[i + 1u] << 4u; // cannot manage odd
        out[i / 2u] = flags[i];
      }
    }

    const u32 remain = size - in_offset;
    if (remain > 0) {
      const u32 flag_idx = out_offset;
      out_offset++; // flags
      if (in_offset == 0) {
        if (in[0] <= 0xFFFFu) {
          reinterpret_cast<u16 *__restrict const>(out + out_offset)[0] =
              static_cast<u16>(in[0]);
          out_offset += 2u;
        } else {
          reinterpret_cast<u32 *__restrict const>(out + out_offset)[0] = in[0];
          out_offset += 4u;
          // nibble from lower bit
          out[flag_idx] = 0b00000001u;
        }
        in_offset++;
      }
      for (; in_offset < size; in_offset++) {
        const u32 diff = in[in_offset] - in[in_offset - 1u];
        if (diff <= 0xFFFFu) {
          reinterpret_cast<u16 *__restrict const>(out + out_offset)[0] =
              static_cast<u16>(diff);
          out_offset += 2u;
        } else {
          reinterpret_cast<u32 *__restrict const>(out + out_offset)[0] = diff;
          out_offset += 4u;
          // nibble from lower bit
          out[flag_idx] |= 0b00000001u << (remain - (size - in_offset));
        }
      }

      out_offset += (8u - remain) * 2u; // skip for overrun in decode
      // if not exists, decoder reads out of byte array
    }

    return out_offset;
  } else {
    return 0;
  }
}
} // namespace hoshizora::compress
#endif // SINGLE_ENCODE_H
