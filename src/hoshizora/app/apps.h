#ifndef HOSHIZORA_APPS_H
#define HOSHIZORA_APPS_H

#include "hoshizora/app/clustering_louvain.h"
#include "hoshizora/app/pagerank.h"
#include "hoshizora/core/bulksync_gas_executor.h"
#include "hoshizora/core/graph.h"
#include "hoshizora/core/includes.h"
#include "hoshizora/core/io.h"
#include <iostream>
#include <map>
#include <set>
#include <utility>

namespace hoshizora {
std::vector<std::string> pagerank(const std::string &file_name,
                                  const u32 num_iters) {
  using _Graph = Graph<u32, u32 /*empty_t*/, empty_t, f32, f32>;
  debug::logger->info("#numa nodes: {}", loop::num_numa_nodes);
  debug::logger->info("#threads: {}", loop::num_threads);
  debug::logger->info("#iters: {}", num_iters);
  debug::point("started");
  auto edge_list = IO::from_file(file_name);
  debug::point("loaded");
  auto graph = _Graph::from_edge_list(edge_list);
  debug::point("converted");
  PageRankKernel<_Graph> kernel{};
  BulkSyncGASExecutor<PageRankKernel<_Graph>> executor(kernel, graph,
                                                       num_iters);
  const auto result = executor.run();
  debug::point("done");

  debug::report("started", "loaded");
  debug::report("loaded", "converted");
  debug::report("converted", "done");
  // loop::quit();

  return result;
}

// FIXME: Just garbage
std::vector<u32> clustering(const std::string &file_name,
                            const u32 num_clusters_hint, const f64 threshold) {
  using G = Graph<u32,                 // ID: cluster_id
                  u32,                 // VProp: e_{ii}
                  u32,                 // EProp: e_{ij}
                  std::pair<u32, f64>, // VData: (new cluster_id, gain)
                  f64>;                // EData: modularity gain
  auto edge_list = IO::from_file(file_name);
  auto graph = G::from_edge_list(edge_list);
  const auto num_vertices = graph.num_vertices;
  // init e_props and cluster_ids
  std::vector<std::unordered_map<G::_ID, G::_EProp>> edge_weights;
  edge_weights.reserve(graph.num_vertices);
  std::vector<u32> cluster_ids{}; // [orig_id] -> cluster_id
  std::map<u32, std::set<u32>>
      inv_cluster_ids{}; // [cluster_id] -> inner_orig_ids[]
  cluster_ids.reserve(graph.num_vertices);
  for (u32 src = 0; src < graph.num_vertices; ++src) {
    cluster_ids.emplace_back(src);
    std::set<u32> s = {src};
    inv_cluster_ids.emplace(src, s);
    edge_weights.emplace_back(std::unordered_map<G::_ID, G::_EProp>());
    for (u32 i = 0, degree = graph.out_degrees(src); i < degree; ++i) {
      const auto dst = graph.out_neighbors(src)[i];
      edge_weights.back().emplace(dst, 1); // let initial edge weight be 1
    }
  }
  graph.num_all_edges = graph.num_edges;
  graph.e_props = edge_weights;
  ClusteringLouvain<G> kernel{threshold};
  BulkSyncGASExecutor<ClusteringLouvain<G>> executor{kernel, graph, 1};
  executor.run();

  // FIXME: Introduce parallel graph compaction API
  std::vector<u32> packed_ids(graph.num_vertices); // [cluster_id] -> packed_id
  std::vector<u32> inv_packed_ids(
      graph.num_vertices); // [packed_id] -> cluster_id
  bool updated = graph.changed;
  while (updated) {
    for (u32 i = 0; i < graph.num_vertices; ++i) {
      const auto curr_cluster_id = i;
      const auto new_cluster_id = graph.v_data(i).first;
      auto inner_node_ids = inv_cluster_ids[i];

      if (*inner_node_ids.begin() == new_cluster_id) {
        for (const auto &inner_node_id : inner_node_ids) {
          cluster_ids[inner_node_id] = new_cluster_id;
        }
      } else {
        inv_cluster_ids.erase(curr_cluster_id);

        for (const auto &inner_node_id : inner_node_ids) {
          cluster_ids[inner_node_id] = new_cluster_id;
          inv_cluster_ids[new_cluster_id].emplace(inner_node_id);
        }
      }
    }

    // smoothing
    bool up = false;
    do {
      up = false;
      for (u32 i = 0; i < graph.num_vertices; ++i) {
        if (cluster_ids[i] != cluster_ids[cluster_ids[i]]) {
          up = true;
          cluster_ids[i] = cluster_ids[cluster_ids[i]];
        }
      }
    } while (up);

    // invert
    std::map<u32, std::set<u32>>
        _inv_cluster_ids{}; // [cluster_id] -> inner_original_ids[]
    for (u32 i = 0; i < graph.num_vertices; ++i) {
      _inv_cluster_ids[cluster_ids[i]].emplace(i);
    }
    u32 packed_id = 0;
    for (const auto &kv : _inv_cluster_ids) {
      packed_ids[kv.first] = packed_id;
      inv_packed_ids[packed_id] = kv.first;
      packed_id++;
    }

    const u32 new_num_vertices = _inv_cluster_ids.size();
    std::vector<std::set<u32>> _adjacency_list{new_num_vertices,
                                               std::set<u32>{}};
    std::vector<std::unordered_map<G::_ID, G::_EProp>> new_edge_weights{
        new_num_vertices, std::unordered_map<G::_ID, G::_EProp>{}};
    std::vector<u32> v_props(new_num_vertices);
    for (const auto &kv : _inv_cluster_ids) {
      const auto src = packed_ids[kv.first];
      for (const auto inner : kv.second) {
        const auto inner_nghs = graph.out_neighbors(inner);
        for (u32 i = 0, deg = graph.out_degrees(inner); i < deg; ++i) {
          const auto ngh = inner_nghs[i];
          const auto ngh_cl = packed_ids[cluster_ids[ngh]];
          if (ngh_cl == src) {
            v_props[src]++;
          } else {
            auto &map = new_edge_weights[src];
            if (map.find(ngh_cl) == map.end()) {
              map.emplace(ngh_cl, 1);
            } else {
              map[ngh_cl]++;
            }
            _adjacency_list[src].emplace(ngh_cl);
          }
        }
      }
    }
    std::vector<std::vector<u32>> adjacency_list{};
    adjacency_list.reserve(new_num_vertices);
    for (const auto &ngh : _adjacency_list) {
      adjacency_list.emplace_back(std::vector<u32>(ngh.begin(), ngh.end()));
    }

    auto num_all_edges = graph.num_all_edges;
    graph = G::from_adjacency_list(adjacency_list); // FIXME: Large memory leak
    graph.e_props = new_edge_weights;
    graph.v_props = v_props;
    graph.num_all_edges = num_all_edges;

    BulkSyncGASExecutor<ClusteringLouvain<G>> _executor(kernel, graph, 1);
    _executor.run();
    updated = new_num_vertices > num_clusters_hint && graph.changed;
  }

  // Output: orig_id -- cluster_id map
  std::vector<u32> res(num_vertices); // [orig_id] -> cluster_id
  for (u32 i = 0; i < num_vertices; ++i) {
    res[i] = packed_ids[cluster_ids[i]];
  }
  return res;
}
} // namespace hoshizora
#endif // HOSHIZORA_APPS_H
