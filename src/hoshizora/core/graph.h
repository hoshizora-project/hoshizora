#ifndef HOSHIZORA_GRAPH_H
#define HOSHIZORA_GRAPH_H

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>
#include <thread>
#include <tuple>
#include <vector>

#include "hoshizora/core/colle.h"
#include "hoshizora/core/includes.h"
#include "hoshizora/core/loop.h"

namespace hoshizora {
/*
 * #blocks = 2
 * data:       [3 | 8 | 2 | 3 | 3 | 6 | 4]
 *             ^0             ^4      ^6 ^7
 * offsets:    [0 | 4 | 6 | 7]
 *              ^0  ^1      ^3
 * boundaries: [0 | 1 | 3]
 *
 * for {
 *   (lower, upper) <- boundaries // (0, 1)
 *   i <- offsets[lower] until offsets[upper] // 0 until 4
 * } {
 *   data[i] // 3, 8, 2, 3
 * }
 */
template <class ID, class VProp, class EProp, class VData, class EData,
          bool IsDirected = true>
struct Graph {
  using _ID = ID;
  using _VProp = VProp;
  using _EProp = EProp;
  using _VData = VData;
  using _EData = EData;
  using _Graph = Graph<ID, VProp, EProp, VData, EData, IsDirected>;

  // TODO
  const u32 num_threads = loop::num_threads;
  const u32 num_numa_nodes = loop::num_numa_nodes;

  ID num_vertices;
  ID num_edges;

  ID *tmp_out_offsets;
  ID *tmp_out_indices;
  ID *tmp_in_offsets;
  ID *tmp_in_indices;

  colle::DiscreteArray<ID> out_degrees;     // [#vertices]
  colle::DiscreteArray<ID> out_offsets;     // [#vertices]
  colle::DiscreteArray<ID *> out_neighbors; // [#vertices][degrees[i]]
  colle::DiscreteArray<ID> in_degrees;
  colle::DiscreteArray<ID> in_offsets;
  colle::DiscreteArray<ID *> in_neighbors;

  // colle::DiscreteArray<u8> out_indices; // [#edges]
  // colle::DiscreteArray<u8> in_indices;  // [#edges]

  colle::DiscreteArray<ID> out_indices; // [#edges]
  colle::DiscreteArray<ID> in_indices;  // [#edges]

  ID *forward_indices; // [num_edges]
  ID *out_boundaries;
  ID *in_boundaries;
  VProp *v_props;                     // [#vertices]
  EProp *e_props;                     // [#edges]
  colle::DiscreteArray<VData> v_data; // [#vertices]
  colle::DiscreteArray<EData> e_data; // [#edges]

  colle::DiscreteArray<bool> active_flags; // [#vertices]

  bool out_degrees_is_initialized = false;
  bool out_offsets_is_initialized = false;
  bool out_indices_is_initialized = false;
  bool in_degrees_is_initialized = false;
  bool in_offsets_is_initialized = false;
  bool in_indices_is_initialized = false;
  bool out_boundaries_is_initialized = false;
  bool in_boundaries_is_initialized = false;
  bool forward_indices_is_initialized = false;

  Graph()
      : out_degrees(colle::DiscreteArray<ID>()),
        out_neighbors(colle::DiscreteArray<ID *>()),
        in_degrees(colle::DiscreteArray<ID>()),
        in_neighbors(colle::DiscreteArray<ID *>()),
        v_data(colle::DiscreteArray<VData>()),
        e_data(colle::DiscreteArray<EData>()) {}

  void set_out_boundaries() {
    assert(!out_offsets_is_initialized);

    const auto chunk_size = num_edges / num_threads;
    out_boundaries = mem::calloc<ID>(num_threads + 1);
    for (u32 thread_id = 1; thread_id < num_threads; ++thread_id) {
      out_boundaries[thread_id] = static_cast<ID>(std::distance(
          tmp_out_offsets,
          std::lower_bound(tmp_out_offsets, tmp_out_offsets + num_vertices,
                           chunk_size * thread_id)));
    }
    out_boundaries[num_threads] = num_vertices;

    out_boundaries_is_initialized = true;
  }

  void set_in_boundaries() {
    assert(!in_offsets_is_initialized);

    const auto chunk_size = num_edges / num_threads;
    in_boundaries = mem::calloc<ID>(num_threads + 1);
    for (u32 thread_id = 1; thread_id < num_threads; ++thread_id) {
      in_boundaries[thread_id] = static_cast<ID>(std::distance(
          tmp_in_offsets,
          std::lower_bound(tmp_in_offsets, tmp_in_offsets + num_vertices,
                           chunk_size * thread_id)));
    }
    in_boundaries[num_threads] = num_vertices;

    in_boundaries_is_initialized = true;
  }

  void set_out_offsets() {
    assert(out_boundaries_is_initialized);

    loop::each_thread(out_boundaries, [&](u32 thread_id, u32 numa_id, ID lower,
                                          ID upper /*, ID acc_num_srcs*/) {
      const auto length = upper - lower + 1; // w/ cap
      const auto offsets = mem::malloc<ID>(length, numa_id);
      std::memcpy(offsets, tmp_out_offsets + lower, length * sizeof(ID));

      // TODO: optimize it
      // if (thread_id > 0) {
      //  const auto offset = offsets[0];
      //  for (ID i = 0; i < length; ++i) {
      //    offsets[i] -= offset;
      //  }
      //}

      out_offsets.add(offsets, length - 1); // real size w/o cap
    });

    mem::free(tmp_out_offsets, sizeof(ID) * num_vertices);
    out_offsets_is_initialized = true;
  }

  void set_in_offsets() {
    assert(in_boundaries_is_initialized);

    loop::each_thread(in_boundaries, [&](u32 thread_id, u32 numa_id, ID lower,
                                         ID upper /*, ID acc_num_srcs*/) {
      const auto length = upper - lower + 1; // w/ cap
      const auto offsets = mem::malloc<ID>(length, numa_id);
      std::memcpy(offsets, tmp_in_offsets + lower, length * sizeof(ID));

      // TODO: optimize it
      // if (thread_id > 0) {
      //  const auto offset = offsets[0];
      //  for (ID i = 0; i < length; ++i) {
      //    offsets[i] -= offset;
      //  }
      //}

      in_offsets.add(offsets, length - 1); // real size w/o cap
    });

    mem::free(tmp_in_offsets, sizeof(ID) * num_vertices);
    in_offsets_is_initialized = true;
  }

  void set_out_degrees() {
    assert(out_boundaries_is_initialized);
    assert(out_offsets_is_initialized);

    loop::each_thread(out_boundaries, [&](u32 thread_id, u32 numa_id, u32 lower,
                                          ID upper /*, ID acc_num_srcs*/) {
      const auto length = upper - lower;
      const auto degrees = mem::malloc<ID>(length, numa_id);
      for (u32 i = lower; i < upper; ++i) {
        degrees[i - lower] =
            out_offsets(i + 1, thread_id) - out_offsets(i, thread_id);
      }
      out_degrees.add(degrees, length);
    });

    out_degrees_is_initialized = true;
  }

  void set_in_degrees() {
    assert(in_boundaries_is_initialized);
    assert(in_offsets_is_initialized);

    loop::each_thread(in_boundaries, [&](u32 thread_id, u32 numa_id, u32 lower,
                                         ID upper /*, ID acc_num_srcs*/) {
      const auto length = upper - lower;
      const auto degrees = mem::malloc<ID>(length, numa_id);
      for (u32 i = lower; i < upper; ++i) {
        degrees[i - lower] =
            in_offsets(i + 1, thread_id) - in_offsets(i, thread_id);
      }
      in_degrees.add(degrees, length);
    });

    in_degrees_is_initialized = true;
  }

  void set_out_indices() {
    assert(out_boundaries_is_initialized);
    assert(out_offsets_is_initialized);

    // const auto *_tmp_out_indices = tmp_out_indices;
    loop::each_thread(out_boundaries, [&](u32 thread_id, u32 numa_id, ID lower,
                                          ID upper /*, ID acc_num_srcs*/) {
      const auto start = out_offsets(lower, thread_id);
      const auto end = out_offsets(upper, thread_id, 0);
      // const auto num_srcs = upper - lower;
      // const auto estimated_size = compress::multiple::estimate(
      //    _tmp_out_indices, out_offsets.data[thread_id], num_srcs);
      // const auto indices = mem::calloc<u8>(estimated_size, numa_id);
      // compress::multiple::encode(_tmp_out_indices,
      // out_offsets.data[thread_id],
      //                           num_srcs, indices);
      const auto num_nghs = end - start;
      const auto indices = mem::calloc<ID>(num_nghs, numa_id);
      std::memcpy(indices, tmp_out_indices + start, num_nghs * sizeof(ID));

      out_indices.add(indices, num_nghs);
      //_tmp_out_indices += end;
    });

    mem::free(tmp_out_indices, num_edges);
    out_indices_is_initialized = true;
  }

  void set_in_indices() {
    assert(in_boundaries_is_initialized);
    assert(in_offsets_is_initialized);

    // const auto *_tmp_in_indices = tmp_in_indices;
    loop::each_thread(in_boundaries, [&](u32 thread_id, u32 numa_id, ID lower,
                                         ID upper /*, ID acc_num_srcs*/) {
      const auto start = in_offsets(lower, thread_id);
      const auto end = in_offsets(upper, thread_id, 0);
      // const auto num_dsts = upper - lower;
      // const auto estimated_size = compress::multiple::estimate(
      //    _tmp_in_indices, in_offsets.data[thread_id], num_dsts);
      // const auto indices = mem::calloc<u8>(estimated_size, numa_id);
      // compress::multiple::encode(_tmp_in_indices, in_offsets.data[thread_id],
      //                           num_dsts, indices);
      const auto num_nghs = end - start;
      const auto indices = mem::calloc<ID>(num_nghs, numa_id);
      std::memcpy(indices, tmp_in_indices + start, num_nghs * sizeof(ID));

      in_indices.add(indices, end - start);
      //_tmp_in_indices += end;
    });

    mem::free(tmp_in_indices, num_edges);
    in_indices_is_initialized = true;
  }

  void set_out_neighbor() {
    assert(out_indices_is_initialized);
    assert(out_offsets_is_initialized);

    loop::each_thread(
        out_boundaries, [&](u32 thread_id, u32 numa_id, u32 lower, u32 upper) {
          auto out_neighbor = colle::make_numa_vector<ID *>(numa_id);
          out_neighbor->reserve(upper - lower);
          for (ID i = lower; i < upper; ++i) {
            out_neighbor->emplace_back(
                &out_indices(out_offsets(i, thread_id), thread_id));
          }
          out_neighbors.add(std::move(*out_neighbor));
        });

    // out_neighbors_is_initialized = true;
  }

  void set_in_neighbor() {
    assert(in_indices_is_initialized);
    assert(in_offsets_is_initialized);

    loop::each_thread(
        in_boundaries, [&](u32 thread_id, u32 numa_id, u32 lower, u32 upper) {
          auto in_neighbor = colle::make_numa_vector<ID *>(numa_id);
          in_neighbor->reserve(upper - lower);
          for (ID i = lower; i < upper; ++i) {
            in_neighbor->emplace_back(
                &in_indices(in_offsets(i, thread_id), thread_id));
          }
          in_neighbors.add(std::move(*in_neighbor));
        });

    // in_neighbors_is_initialized = true;
  }

  void set_forward_indices() {
    assert(out_boundaries_is_initialized);
    assert(out_offsets_is_initialized);
    assert(out_degrees_is_initialized);
    assert(out_indices_is_initialized);
    assert(in_offsets_is_initialized);

    forward_indices = mem::malloc<ID>(num_edges); // TODO: should be numa-local

    auto counts = std::vector<ID>(num_vertices, 0);
    // loop::each_index(
    //    out_boundaries, out_indices, out_offsets,
    //    [&](u32 thread_id, u32 numa_id, ID dst, ID local_offset, ID
    //    global_idx,
    //        ID local_idx, ID global_offset) {
    //      forward_indices[global_offset + local_offset + local_idx] =
    //          in_offsets(dst) + counts[dst];
    //      counts[dst]++;
    //    });

    loop::each_thread(
        out_boundaries, [&](u32 thread_id, u32 numa_id, u32 lower, u32 upper) {
          ;
          for (ID src = lower; src < upper; ++src) {
            const auto neighbor = out_neighbors(src, thread_id);
            for (ID i = 0, end = out_degrees(src, thread_id); i < end; ++i) {
              const auto dst = neighbor[i];
              forward_indices[out_offsets(src, thread_id) + i] =
                  in_offsets(dst) + counts[dst];
              counts[dst]++;
            }
          }
        });

    forward_indices_is_initialized = true;
  }

  void set_v_data(bool allow_overwrite = false) {
    assert(out_boundaries_is_initialized);
    // assert(in_boundaries_is_initialized);

    if (allow_overwrite) {
      v_data = *(new colle::DiscreteArray<VData>());
    }

    // TODO: consider both out and in boundaries (?)
    // If readonly, it should be allowed that duplicate vertex data
    // And should be allocated on each numa node
    loop::each_thread(out_boundaries, [&](u32 thread_id, u32 numa_id, ID lower,
                                          ID upper /*, ID acc_num_srcs*/) {
      const auto num_inner_vertices = upper - lower;
      v_data.add(mem::malloc<VData>(num_inner_vertices, numa_id),
                 num_inner_vertices);
    });
  }

  void set_e_data(bool allow_overwirte = false) {
    assert(out_boundaries_is_initialized);
    assert(out_offsets_is_initialized);
    // assert(in_boundaries_is_initialized);
    // assert(in_offsets_is_initialized);

    if (allow_overwirte) {
      e_data = *(new colle::DiscreteArray<EData>());
    }

    // TODO: consider both out and in boundaries (?)
    // If readonly, it should be allowed that duplicate edge data
    // And should be allocated on each numa node
    loop::each_thread(out_boundaries, [&](u32 thread_id, u32 numa_id, ID lower,
                                          ID upper /*, ID acc_num_srcs*/) {
      const auto start = out_offsets(lower, thread_id);
      const auto end = out_offsets(upper, thread_id);
      const auto num_inner_edges = end - start;
      e_data.add(mem::malloc<VData>(num_inner_edges, numa_id), num_inner_edges);
    });
  }

  static void Next(_Graph &prev, _Graph &curr) {
    // TODO
    std::swap(prev.v_data, curr.v_data);
    std::swap(prev.e_data, curr.e_data);
  }

  // TODO: poor performance
  // *require packed index* (process in preprocessing)
  static _Graph from_edge_list(const std::pair<ID, ID> *edge_list, size_t len) {
    assert(len > 0 && edge_list != nullptr);

    using VVType = std::pair<ID, ID>;
    auto vec = std::vector<VVType>(edge_list, edge_list + len); // [(from, to)]

    // outbound
    std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
      return l.second < r.second;
    });
    auto tmp_max = vec.back().second;
    std::stable_sort(
        begin(vec), end(vec),
        [](const VVType &l, const VVType &r) { return l.first < r.first; });

    auto num_vertices = std::max(tmp_max, vec.back().first) + 1; // 0-based
    auto num_edges = len;

    auto out_offsets = mem::malloc<ID>(num_vertices + 1); // all vertices + cap
    auto out_indices = mem::malloc<ID>(num_edges);        // all edges

    out_offsets[0] = 0u;
    auto prev_src = vec[0].first;
    for (u32 i = 0; i < num_edges; ++i) {
      auto curr = vec[i];
      // next source vertex
      if (prev_src != curr.first) {
        // loop for vertices whose degree is 0
        for (auto j = prev_src + 1; j <= curr.first; ++j) {
          out_offsets[j] = i;
        }
        prev_src = curr.first;
      }
      out_indices[i] = curr.second;
    }
    for (auto i = prev_src + 1; i <= num_vertices; ++i) {
      out_offsets[i] = num_edges;
    }

    // and vice versa (inbound)
    for (auto &p : vec) {
      std::swap(p.first, p.second);
    }
    std::sort(begin(vec), end(vec), [](const VVType &l, const VVType &r) {
      return l.second < r.second;
    });
    std::stable_sort(
        begin(vec), end(vec),
        [](const VVType &l, const VVType &r) { return l.first < r.first; });

    auto in_offsets = mem::malloc<ID>(num_vertices);
    auto in_indices = mem::malloc<ID>(num_edges);

    in_offsets[0] = 0;
    prev_src = vec[0].first;
    for (u32 i = 0; i < num_edges; ++i) {
      auto curr = vec[i];
      if (prev_src != curr.first) {
        // loop for vertices whose degree is 0
        for (auto j = prev_src + 1; j <= curr.first; ++j) {
          in_offsets[j] = i;
        }
        prev_src = curr.first;
      }
      in_indices[i] = curr.second;
    }
    for (auto i = prev_src + 1; i <= num_vertices; ++i) {
      in_offsets[i] = num_edges;
    }

    auto g = _Graph();
    g.num_vertices = num_vertices;
    g.num_edges = num_edges;
    g.tmp_out_offsets = out_offsets;
    g.tmp_out_indices = out_indices;
    g.tmp_in_offsets = in_offsets;
    g.tmp_in_indices = in_indices;
    g.set_out_boundaries();
    g.set_out_offsets();
    g.set_out_degrees();
    g.set_out_indices();
    g.set_out_neighbor();
    g.set_in_boundaries();
    g.set_in_offsets();
    g.set_in_degrees();
    g.set_in_indices();
    g.set_in_neighbor();
    g.set_forward_indices();
    g.set_v_data();
    g.set_e_data();

    assert(g.out_degrees_is_initialized);
    assert(g.out_offsets_is_initialized);
    assert(g.out_indices_is_initialized);
    assert(g.in_degrees_is_initialized);
    assert(g.in_offsets_is_initialized);
    assert(g.in_indices_is_initialized);
    assert(g.out_boundaries_is_initialized);
    assert(g.in_boundaries_is_initialized);
    assert(g.forward_indices_is_initialized);

    return g;
  }
};
} // namespace hoshizora

#endif // HOSHIZORA_GRAPH_H
