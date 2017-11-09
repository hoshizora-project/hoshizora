#ifndef HOSHIZORA_COLLE_H
#define HOSHIZORA_COLLE_H

#include "hoshizora/core/compress/single.h"
#include "hoshizora/core/util/includes.h"
#include <vector>

namespace hoshizora::colle {
template <class T> static inline numa_vector<T> *make_numa_vector(u32 numa_id) {
#ifdef SUPPORT_NUMA
  return new numa_vector<T>(*(mem::allocators[numa_id]));
#else
  return new numa_vector<T>();
#endif
}

// TODO: SIMD-aware
template <class T> struct DiscreteArray {
  // TODO: Redundant on each numa node
  std::vector<T *> data;
  std::vector<u32> range;

  DiscreteArray() { range.emplace_back(0); }

  DiscreteArray(std::vector<T *> &data, std::vector<u32> &range)
      : data(data), range(range) {}

  u64 size() { return data.size(); }

  void add(T *datum, u32 length) {
    data.emplace_back(datum);
    range.emplace_back(range.back() + length);
  }

  void add(numa_vector<T> &chunk) {
    data.emplace_back(chunk.data());
    range.emplace_back(range.back() + chunk.size());
  }

  void add(numa_vector<T> &&chunk) {
    data.emplace_back(chunk.data());
    range.emplace_back(range.back() + chunk.size());
  }

  // significantly slower than normal index access on a single malloc
  //[[deprecated("Recommended to call with hint")]]
  T &operator()(u32 index) {
    // TODO: sequential search may be faster
    const auto n = std::distance(begin(range) + 1,
                                 upper_bound(begin(range), end(range), index));
    return data[n][index - range[n]];
  }

  T &operator()(u32 index, void *dummy) const {
    // TODO: sequential search may be faster
    const auto n = std::distance(begin(range) + 1,
                                 upper_bound(begin(range), end(range), index));
    return data[n][index - range[n]];
  }

  // TODO
  // faster than normal index access on a single malloc
  T operator()(u32 index, u32 n, u32 dummy) const {
    // if constexpr (support_numa) data[n][index - range[n]] else
    // data[0][index];
    return data[n][index - range[n]];
  }

  // TODO
  // faster than normal index access on a single malloc
  T &operator()(u32 index, u32 n) {
    // if constexpr (support_numa) data[n][index - range[n]] else
    // data[0][index];
    return data[n][index - range[n]];
  }

  template <
      class
      Func /*(unpacked_datum, local_offset, global_idx, local_idx, global_offset)*/>
  void foreach (u32 thread_id, u32 dummy, Func f) const {
    // this type manages a single array like [0,1,3,5,2,1]
    for (u32 i = 0, offset = range[thread_id],
             end = range[thread_id + 1] - offset;
         i < end; ++i) {
      f(data[thread_id][i], // el of array
        0, // offset of array, always 0, since it contains just a single array
        0, // index of array, always 0, since it contains just a single array
        i, // index of el
        offset); // offset of array, which is useful for outside
    }
  }
};

template <>
template <
    class
    Func /*(unpacked_datum, local_offset, global_idx, local_idx, global_offset)*/>
void DiscreteArray<u8>::foreach (u32 thread_id, u32 num_inner_lists,
                                 Func f) const {
  // this type manages multiple array in a single array like [0,4,2,1,3] as
  // [[0,4,2], [1,3]]
  const auto global_offset = range[thread_id];
  compress::multiple::foreach (
      data[thread_id], num_inner_lists,
      [=, &f](u32 i, u32 local_offset, u32 _global_idx, u32 local_idx) {
        f(i,              // el of array (dst)
          local_offset,   // offset of each inner array
          _global_idx,    // index of inner array (src)
          local_idx,      // index of el in inner array
          global_offset); // offset of array, which is useful for outside
      });
}
} // namespace hoshizora::colle
#endif // HOSHIZORA_COLLE_H
