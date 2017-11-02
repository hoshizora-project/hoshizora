#ifndef HOSHIZORA_NUMA_ALLOCATOR_H
#define HOSHIZORA_NUMA_ALLOCATOR_H

#ifdef SUPPORT_NUMA
#include <new>
#include <numa.h>
#include <type_traits>

#include "hoshizora/core/util/includes.h"

namespace hoshizora {
template <class T> class NumaAllocator {
public:
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  const u32 node;

  explicit NumaAllocator(u32 node = 0) noexcept : node(node){};

  T *allocate(size_t num) {
    auto ret = numa_alloc_onnode(num * sizeof(T), node);
    if (!ret)
      throw std::bad_alloc();
    return reinterpret_cast<T *>(ret);
  }

  void deallocate(T *p, size_t num) noexcept { numa_free(p, num * sizeof(T)); }
};

template <class T1, class T2>
bool operator==(const NumaAllocator<T1> &, const NumaAllocator<T2> &) noexcept {
  return true;
}

template <class T1, class T2>
bool operator!=(const NumaAllocator<T1> &, const NumaAllocator<T2> &) noexcept {
  return false;
}

template <class T> class NumaAllocatorLocal {
public:
  using value_type = T;
  using propagate_on_container_move_assignment = std::true_type;
  using is_always_equal = std::true_type;

  explicit NumaAllocatorLocal() noexcept {};

  T *allocate(size_t num) {
    auto ret = numa_alloc_local(num * sizeof(T));
    if (!ret)
      throw std::bad_alloc();
    return reinterpret_cast<T *>(ret);
  }

  void deallocate(T *p, size_t num) noexcept { numa_free(p, num * sizeof(T)); }
};

template <class T1, class T2>
bool operator==(const NumaAllocatorLocal<T1> &,
                const NumaAllocatorLocal<T2> &) noexcept {
  return true;
}

template <class T1, class T2>
bool operator!=(const NumaAllocatorLocal<T1> &,
                const NumaAllocatorLocal<T2> &) noexcept {
  return false;
}
} // namespace hoshizora
#endif

#endif // HOSHIZORA_NUMA_ALLOCATOR_H
