#ifndef COMPRESS_COMMON
#define COMPRESS_COMMON

#include <cstdint>
#include <stdlib.h>
#include <vector>
#include <immintrin.h>

using namespace std;

/*
 * const value
 */
constexpr auto BIT_PER_BYTE = 8u;
constexpr auto YMM_BIT = 256u;
constexpr auto YMM_BYTE = YMM_BIT / BIT_PER_BYTE;
constexpr auto BITSIZEOF_T = sizeof(uint32_t) * BIT_PER_BYTE;
constexpr auto LENGTH = YMM_BIT / BITSIZEOF_T;
constexpr auto BIT_PER_BOX = YMM_BIT / LENGTH;



/*
 * synonym
 */
using __m256ipc = __m256i *__restrict const;
using __m256icpc = const __m256i *__restrict const;



/*
* shift and mask
*/
constexpr uint32_t shift8r(const uint8_t count) {
  const auto x = 0xFFu >> count;
  return x | x << 8u | x << 16u | x << 24u;
}

constexpr uint32_t shift8l(const uint8_t count) {
  const auto x = 0xFFu << count;
  return x | x << 8u | x << 16u | x << 24u;
}

constexpr uint32_t shift32r(const uint8_t count) {
  return 0xFFFFFFFFu >> count;
}

constexpr uint32_t shift32l(const uint8_t count) {
  return 0xFFFFFFFFu << count;
}

alignas(32) constexpr uint32_t mask8r[9][8] = {
  { shift8r(8), shift8r(8), shift8r(8), shift8r(8), shift8r(8), shift8r(8), shift8r(8), shift8r(8) },
  { shift8r(7), shift8r(7), shift8r(7), shift8r(7), shift8r(7), shift8r(7), shift8r(7), shift8r(7) },
  { shift8r(6), shift8r(6), shift8r(6), shift8r(6), shift8r(6), shift8r(6), shift8r(6), shift8r(6) },
  { shift8r(5), shift8r(5), shift8r(5), shift8r(5), shift8r(5), shift8r(5), shift8r(5), shift8r(5) },
  { shift8r(4), shift8r(4), shift8r(4), shift8r(4), shift8r(4), shift8r(4), shift8r(4), shift8r(4) },
  { shift8r(3), shift8r(3), shift8r(3), shift8r(3), shift8r(3), shift8r(3), shift8r(3), shift8r(3) },
  { shift8r(2), shift8r(2), shift8r(2), shift8r(2), shift8r(2), shift8r(2), shift8r(2), shift8r(2) },
  { shift8r(1), shift8r(1), shift8r(1), shift8r(1), shift8r(1), shift8r(1), shift8r(1), shift8r(1) },
  { shift8r(0), shift8r(0), shift8r(0), shift8r(0), shift8r(0), shift8r(0), shift8r(0), shift8r(0) }
};

alignas(32) constexpr uint32_t mask8l[9][8] = {
  { shift8l(8), shift8l(8), shift8l(8), shift8l(8), shift8l(8), shift8l(8), shift8l(8), shift8l(8) },
  { shift8l(7), shift8l(7), shift8l(7), shift8l(7), shift8l(7), shift8l(7), shift8l(7), shift8l(7) },
  { shift8l(6), shift8l(6), shift8l(6), shift8l(6), shift8l(6), shift8l(6), shift8l(6), shift8l(6) },
  { shift8l(5), shift8l(5), shift8l(5), shift8l(5), shift8l(5), shift8l(5), shift8l(5), shift8l(5) },
  { shift8l(4), shift8l(4), shift8l(4), shift8l(4), shift8l(4), shift8l(4), shift8l(4), shift8l(4) },
  { shift8l(3), shift8l(3), shift8l(3), shift8l(3), shift8l(3), shift8l(3), shift8l(3), shift8l(3) },
  { shift8l(2), shift8l(2), shift8l(2), shift8l(2), shift8l(2), shift8l(2), shift8l(2), shift8l(2) },
  { shift8l(1), shift8l(1), shift8l(1), shift8l(1), shift8l(1), shift8l(1), shift8l(1), shift8l(1) },
  { shift8l(0), shift8l(0), shift8l(0), shift8l(0), shift8l(0), shift8l(0), shift8l(0), shift8l(0) }
};

// TODO: not know why but use of constexpr here makes slow
alignas(32) const uint32_t mask32r[33][8] = {
  { shift32r(32), shift32r(32), shift32r(32), shift32r(32), shift32r(32), shift32r(32), shift32r(32), shift32r(32) },
  { shift32r(31), shift32r(31), shift32r(31), shift32r(31), shift32r(31), shift32r(31), shift32r(31), shift32r(31) },
  { shift32r(30), shift32r(30), shift32r(30), shift32r(30), shift32r(30), shift32r(30), shift32r(30), shift32r(30) },
  { shift32r(29), shift32r(29), shift32r(29), shift32r(29), shift32r(29), shift32r(29), shift32r(29), shift32r(29) },
  { shift32r(28), shift32r(28), shift32r(28), shift32r(28), shift32r(28), shift32r(28), shift32r(28), shift32r(28) },
  { shift32r(27), shift32r(27), shift32r(27), shift32r(27), shift32r(27), shift32r(27), shift32r(27), shift32r(27) },
  { shift32r(26), shift32r(26), shift32r(26), shift32r(26), shift32r(26), shift32r(26), shift32r(26), shift32r(26) },
  { shift32r(25), shift32r(25), shift32r(25), shift32r(25), shift32r(25), shift32r(25), shift32r(25), shift32r(25) },
  { shift32r(24), shift32r(24), shift32r(24), shift32r(24), shift32r(24), shift32r(24), shift32r(24), shift32r(24) },
  { shift32r(23), shift32r(23), shift32r(23), shift32r(23), shift32r(23), shift32r(23), shift32r(23), shift32r(23) },
  { shift32r(22), shift32r(22), shift32r(22), shift32r(22), shift32r(22), shift32r(22), shift32r(22), shift32r(22) },
  { shift32r(21), shift32r(21), shift32r(21), shift32r(21), shift32r(21), shift32r(21), shift32r(21), shift32r(21) },
  { shift32r(20), shift32r(20), shift32r(20), shift32r(20), shift32r(20), shift32r(20), shift32r(20), shift32r(20) },
  { shift32r(19), shift32r(19), shift32r(19), shift32r(19), shift32r(19), shift32r(19), shift32r(19), shift32r(19) },
  { shift32r(18), shift32r(18), shift32r(18), shift32r(18), shift32r(18), shift32r(18), shift32r(18), shift32r(18) },
  { shift32r(17), shift32r(17), shift32r(17), shift32r(17), shift32r(17), shift32r(17), shift32r(17), shift32r(17) },
  { shift32r(16), shift32r(16), shift32r(16), shift32r(16), shift32r(16), shift32r(16), shift32r(16), shift32r(16) },
  { shift32r(15), shift32r(15), shift32r(15), shift32r(15), shift32r(15), shift32r(15), shift32r(15), shift32r(15) },
  { shift32r(14), shift32r(14), shift32r(14), shift32r(14), shift32r(14), shift32r(14), shift32r(14), shift32r(14) },
  { shift32r(13), shift32r(13), shift32r(13), shift32r(13), shift32r(13), shift32r(13), shift32r(13), shift32r(13) },
  { shift32r(12), shift32r(12), shift32r(12), shift32r(12), shift32r(12), shift32r(12), shift32r(12), shift32r(12) },
  { shift32r(11), shift32r(11), shift32r(11), shift32r(11), shift32r(11), shift32r(11), shift32r(11), shift32r(11) },
  { shift32r(10), shift32r(10), shift32r(10), shift32r(10), shift32r(10), shift32r(10), shift32r(10), shift32r(10) },
  { shift32r(9), shift32r(9), shift32r(9), shift32r(9), shift32r(9), shift32r(9), shift32r(9), shift32r(9) },
  { shift32r(8), shift32r(8), shift32r(8), shift32r(8), shift32r(8), shift32r(8), shift32r(8), shift32r(8) },
  { shift32r(7), shift32r(7), shift32r(7), shift32r(7), shift32r(7), shift32r(7), shift32r(7), shift32r(7) },
  { shift32r(6), shift32r(6), shift32r(6), shift32r(6), shift32r(6), shift32r(6), shift32r(6), shift32r(6) },
  { shift32r(5), shift32r(5), shift32r(5), shift32r(5), shift32r(5), shift32r(5), shift32r(5), shift32r(5) },
  { shift32r(4), shift32r(4), shift32r(4), shift32r(4), shift32r(4), shift32r(4), shift32r(4), shift32r(4) },
  { shift32r(3), shift32r(3), shift32r(3), shift32r(3), shift32r(3), shift32r(3), shift32r(3), shift32r(3) },
  { shift32r(2), shift32r(2), shift32r(2), shift32r(2), shift32r(2), shift32r(2), shift32r(2), shift32r(2) },
  { shift32r(1), shift32r(1), shift32r(1), shift32r(1), shift32r(1), shift32r(1), shift32r(1), shift32r(1) },
  { shift32r(0), shift32r(0), shift32r(0), shift32r(0), shift32r(0), shift32r(0), shift32r(0), shift32r(0) }
};

/*
alignas(32) const uint32_t mask32l[33][8] = {
  { shift32l(32), shift32l(32), shift32l(32), shift32l(32), shift32l(32), shift32l(32), shift32l(32), shift32l(32) },
  { shift32l(31), shift32l(31), shift32l(31), shift32l(31), shift32l(31), shift32l(31), shift32l(31), shift32l(31) },
  { shift32l(30), shift32l(30), shift32l(30), shift32l(30), shift32l(30), shift32l(30), shift32l(30), shift32l(30) },
  { shift32l(29), shift32l(29), shift32l(29), shift32l(29), shift32l(29), shift32l(29), shift32l(29), shift32l(29) },
  { shift32l(28), shift32l(28), shift32l(28), shift32l(28), shift32l(28), shift32l(28), shift32l(28), shift32l(28) },
  { shift32l(27), shift32l(27), shift32l(27), shift32l(27), shift32l(27), shift32l(27), shift32l(27), shift32l(27) },
  { shift32l(26), shift32l(26), shift32l(26), shift32l(26), shift32l(26), shift32l(26), shift32l(26), shift32l(26) },
  { shift32l(25), shift32l(25), shift32l(25), shift32l(25), shift32l(25), shift32l(25), shift32l(25), shift32l(25) },
  { shift32l(24), shift32l(24), shift32l(24), shift32l(24), shift32l(24), shift32l(24), shift32l(24), shift32l(24) },
  { shift32l(23), shift32l(23), shift32l(23), shift32l(23), shift32l(23), shift32l(23), shift32l(23), shift32l(23) },
  { shift32l(22), shift32l(22), shift32l(22), shift32l(22), shift32l(22), shift32l(22), shift32l(22), shift32l(22) },
  { shift32l(21), shift32l(21), shift32l(21), shift32l(21), shift32l(21), shift32l(21), shift32l(21), shift32l(21) },
  { shift32l(20), shift32l(20), shift32l(20), shift32l(20), shift32l(20), shift32l(20), shift32l(20), shift32l(20) },
  { shift32l(19), shift32l(19), shift32l(19), shift32l(19), shift32l(19), shift32l(19), shift32l(19), shift32l(19) },
  { shift32l(18), shift32l(18), shift32l(18), shift32l(18), shift32l(18), shift32l(18), shift32l(18), shift32l(18) },
  { shift32l(17), shift32l(17), shift32l(17), shift32l(17), shift32l(17), shift32l(17), shift32l(17), shift32l(17) },
  { shift32l(16), shift32l(16), shift32l(16), shift32l(16), shift32l(16), shift32l(16), shift32l(16), shift32l(16) },
  { shift32l(15), shift32l(15), shift32l(15), shift32l(15), shift32l(15), shift32l(15), shift32l(15), shift32l(15) },
  { shift32l(14), shift32l(14), shift32l(14), shift32l(14), shift32l(14), shift32l(14), shift32l(14), shift32l(14) },
  { shift32l(13), shift32l(13), shift32l(13), shift32l(13), shift32l(13), shift32l(13), shift32l(13), shift32l(13) },
  { shift32l(12), shift32l(12), shift32l(12), shift32l(12), shift32l(12), shift32l(12), shift32l(12), shift32l(12) },
  { shift32l(11), shift32l(11), shift32l(11), shift32l(11), shift32l(11), shift32l(11), shift32l(11), shift32l(11) },
  { shift32l(10), shift32l(10), shift32l(10), shift32l(10), shift32l(10), shift32l(10), shift32l(10), shift32l(10) },
  { shift32l(9), shift32l(9), shift32l(9), shift32l(9), shift32l(9), shift32l(9), shift32l(9), shift32l(9) },
  { shift32l(8), shift32l(8), shift32l(8), shift32l(8), shift32l(8), shift32l(8), shift32l(8), shift32l(8) },
  { shift32l(7), shift32l(7), shift32l(7), shift32l(7), shift32l(7), shift32l(7), shift32l(7), shift32l(7) },
  { shift32l(6), shift32l(6), shift32l(6), shift32l(6), shift32l(6), shift32l(6), shift32l(6), shift32l(6) },
  { shift32l(5), shift32l(5), shift32l(5), shift32l(5), shift32l(5), shift32l(5), shift32l(5), shift32l(5) },
  { shift32l(4), shift32l(4), shift32l(4), shift32l(4), shift32l(4), shift32l(4), shift32l(4), shift32l(4) },
  { shift32l(3), shift32l(3), shift32l(3), shift32l(3), shift32l(3), shift32l(3), shift32l(3), shift32l(3) },
  { shift32l(2), shift32l(2), shift32l(2), shift32l(2), shift32l(2), shift32l(2), shift32l(2), shift32l(2) },
  { shift32l(1), shift32l(1), shift32l(1), shift32l(1), shift32l(1), shift32l(1), shift32l(1), shift32l(1) },
  { shift32l(0), shift32l(0), shift32l(0), shift32l(0), shift32l(0), shift32l(0), shift32l(0), shift32l(0) }
};
*/

alignas(32) constexpr uint32_t _broadcast_mask[8] = { 7, 7, 7, 7, 7, 7, 7, 7 };
const auto broadcast_mask = _mm256_load_si256(reinterpret_cast<__m256icpc>(_broadcast_mask));



/*
* intrinsics helper
*/
static inline __m256i _mm256_srli_epi8(const __m256i a, const uint32_t count) {
  return _mm256_and_si256(
    _mm256_load_si256(reinterpret_cast<__m256icpc>(mask8r[count])),
    _mm256_srli_epi32(a, count));
}

static inline __m256i _mm256_slli_epi8(const __m256i a, const uint32_t count) {
  return _mm256_and_si256(
    _mm256_load_si256(reinterpret_cast<__m256icpc>(mask8l[count])),
    _mm256_slli_epi32(a, count));
}



/*
 * flag pack
 */
constexpr uint32_t pack_sizes[16] = { 3, 5, 6, 7, 8, 9, 10, 11, 12, 14, 16, 19, 22, 25, 28, 32 };
constexpr uint8_t pack_sizes_helper[32] = {
  /* 32 */ 15, 15, 15, 15,
  /* 28 */ 14, 14, 14,
  /* 25 */ 13, 13, 13,
  /* 22 */ 12, 12, 12,
  /* 19 */ 11, 11, 11,
  /* 16 */ 10, 10,
  /* 14 */ 9, 9,
  /* 12 */ 8,
  /* 11 */ 7,
  /* 10 */ 6,
  /*  9 */ 5,
  /*  8 */ 4,
  /*  7 */ 3,
  /*  6 */ 2,
  /*  5 */ 1, 1,
  /*  3 */ 0, 0, 0
};



/*
 * aligned vector
 */
template<size_t N>
struct aligned {
  template<typename T>
  struct allocator {
    using value_type = T;

    allocator() {}

    template<typename U>
    allocator(const aligned<N>::allocator<U>&) {}

    T *allocate(size_t n) {
      return reinterpret_cast<T *>(std::malloc(sizeof(T) * n));
      //return reinterpret_cast<T *>(aligned_alloc(N, sizeof(T) * n));
    }

    void deallocate(T *p, size_t n) {
      free(p);
    }
  };
};

template<typename T, size_t N>
using aligned_vector = vector<T, aligned<32>::allocator<T>>; // TODO

template<typename T>
using a32_vector = aligned_vector<T, 32>;

#endif // COMPRESS_COMMON
