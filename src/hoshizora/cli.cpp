#include "hoshizora/app/pagerank.h"
#include "hoshizora/core/executor/bulksync_gas_executor.h"
#include "hoshizora/core/io/io.h"
#include "hoshizora/core/model/graph.h"
#include "hoshizora/core/util/includes.h"
#include <iostream>
#include <utility>

namespace hoshizora {
void main(int argc, char *argv[]) {
  using _Graph = Graph<u32, empty_t, empty_t, f32, f32>;
  init();
  const u32 num_iters =
      argc > 2 ? (u32)std::strtol(argv[2], nullptr, 10) : 1000;
  debug::logger->info("#numa nodes: {}", loop::num_numa_nodes);
  debug::logger->info("#threads: {}", loop::num_threads);
  debug::logger->info("#iters: {}", num_iters);
  debug::point("started");
  auto edge_list = IO::fromFile0(argv[1]);
  debug::point("loaded");
  auto graph = _Graph::from_edge_list(edge_list.data(), edge_list.size());
  debug::point("converted");
  BulkSyncGASExecutor<PageRankKernel<_Graph>> executor(graph, num_iters);
  const auto result = executor.run();
  debug::point("done");
  for (const auto &res : result) {
    printf("%s\n", res.c_str());
  }

  debug::report("started", "loaded");
  debug::report("loaded", "converted");
  debug::report("converted", "done");
  // loop::quit();
}
} // namespace hoshizora

int main(int argc, char *argv[]) {
  hoshizora::main(argc, argv);
  return 0;
}
