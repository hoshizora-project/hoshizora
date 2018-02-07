#ifndef HOSHIZORA_PAGERANK_H
#define HOSHIZORA_PAGERANK_H

#include "hoshizora/core/includes.h"
#include "hoshizora/core/kernel.h"
#include <string>

namespace hoshizora {
template <class Graph> struct PageRankKernel : Kernel<Graph> {
  using _Graph = Graph;
  using EData = typename Graph::_EData;
  using VData = typename Graph::_VData;
  using ID = typename Graph::_ID;

  constexpr static auto JUMP_PROB = 0.15;

  VData init(const ID src, const Graph &graph) const {
    //            return 1.0 / graph.num_vertices;
    return 1.0;
  }

  EData scatter(const ID src, const ID dst, const VData v_val, Graph &graph) {
    return v_val / graph.out_degrees(src, nullptr); // TODO: numa_id
  }

  EData gather(const ID src, const ID dst, const VData prev_val,
               const VData curr_val, const Graph &graph) const {
    return curr_val;
  }

  VData zero(const ID dst, const Graph &graph) const { return 0.0; }

  VData sum(const ID src, const ID dst, const VData v_val, const EData e_val,
            Graph &graph) {
    return v_val + e_val;
  }

  VData apply(const ID dst, const VData prev_val, const VData curr_val,
              const Graph &graph) const {
    return (1 - JUMP_PROB) * curr_val + JUMP_PROB / graph.num_vertices;
  }

  std::vector<std::string> result(const Graph &graph) const {
    std::vector<std::string> results{};
    results.reserve(graph.num_vertices);
    for (size_t i = 0; i < graph.num_vertices; ++i) {
      results.emplace_back(std::to_string(graph.v_data(i)));
    }
    return results;
  }
};
} // namespace hoshizora

#endif // HOSHIZORA_PAGERANK_H
