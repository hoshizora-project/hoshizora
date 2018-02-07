#ifndef HOSHIZORA_CLUSTERING_NEWMAN_H
#define HOSHIZORA_CLUSTERING_NEWMAN_H

#include "hoshizora/core/includes.h"
#include "hoshizora/core/kernel.h"
#include <string>
#include <vector>

namespace hoshizora {
// Like louvain, but more more stupid
template <class Graph> struct ClusteringLouvain : Kernel<Graph> {
  using _Graph = Graph;
  using EData = typename Graph::_EData;
  using VData = typename Graph::_VData;
  using ID = typename Graph::_ID;

  const f64 threshold;

  ClusteringLouvain(const f64 threshold) : threshold(threshold) {}

  VData init(const ID src, const Graph &graph) const override {
    return std::make_pair(src, 0);
  }

  // TODO: Introduce scatter_all
  EData scatter(const ID src, const ID dst, const VData v_val,
                Graph &graph) override {
    // q_{src}
    // Need only the beginning(=|V| times), but currently called |E| times
    u32 sum = graph.v_props.empty() ? 0 : graph.v_props[src];
    for (u32 i = 0, degree = graph.out_degrees(src); i < degree; ++i) {
      const auto ngh = graph.out_neighbors(src)[i];
      sum += graph.e_props[src].at(ngh);
    }
    for (u32 i = 0, deg = graph.in_degrees(src); i < deg; ++i) {
      const auto ngh = graph.in_neighbors(src)[i];
      sum += graph.e_props[ngh].at(src);
    }
    const f64 q = sum / (2.0 * graph.num_all_edges);
    graph.v_data(src) = std::make_pair(src, q);
    return q;
  }

  EData gather(const ID src, const ID dst, const EData prev_val,
               const EData curr_val /*q_{src}*/,
               const Graph &graph) const override {
    // if no outgoing edge, not initialize at scatter
    if (graph.out_degrees(dst) == 0) {
      u32 sum = graph.v_props.empty() ? 0 : graph.v_props[dst];
      for (u32 i = 0, deg = graph.in_degrees(dst); i < deg; ++i) {
        const auto ngh = graph.in_neighbors(dst)[i];
        sum += graph.e_props[ngh].at(dst);
      }
      const f64 q = sum / (2.0 * graph.num_all_edges);
      graph.v_data(dst) = std::make_pair(dst, q);
    }

    return 2 * (graph.e_props[src].at(dst) / (2.0 * graph.num_all_edges) -
                curr_val * graph.v_data(dst).second);
  }

  VData zero(const ID dst, const Graph &graph) const override {
    return std::make_pair(dst, threshold);
  }

  VData sum(const ID src, const ID dst, const VData v_val /*gain_{src, dst}*/,
            const EData e_val, Graph &graph) override {
    if (e_val > v_val.second) {
      graph.changed = true;
      u32 new_cluster_id = std::min(src, dst);
      return std::make_pair(new_cluster_id, e_val);
    } else {
      return v_val;
    }
  }

  VData apply(const ID dst, const VData prev_val, const VData curr_val,
              const Graph &graph) const override {
    return curr_val;
  }

  std::vector<std::string> result(const Graph &graph) const override {
    std::vector<std::string> result;
    return result;
  }
};
} // namespace hoshizora

#endif // HOSHIZORA_CLUSTERING_NEWMAN_H
