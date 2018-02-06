#include "hoshizora/app/pagerank.h"
#include "hoshizora/core/executor/bulksync_gas_executor.h"
#include "hoshizora/core/io/io.h"
#include "hoshizora/core/util/includes.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace hoshizora {
PYBIND11_MODULE(hoshizora, m) {
  namespace py = pybind11;

  m.doc() = "hoshizora: Fast graph analysis engine";
  m.def("pagerank", [](const std::string &file, const u32 num_iters) {
    auto edge_list = IO::fromFile0(file);
    using _Graph = Graph<u32, empty_t, empty_t, f32, f32>;
    auto graph = _Graph::from_edge_list(edge_list.data(), edge_list.size());
    BulkSyncGASExecutor<PageRankKernel<_Graph>> executor(graph, num_iters);
    return executor.run();
  });
}
} // namespace hoshizora