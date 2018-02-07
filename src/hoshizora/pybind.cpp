#include "hoshizora/app/apps.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace hoshizora {
PYBIND11_MODULE(hoshizora, m) {
  namespace py = pybind11;

  m.doc() = "hoshizora: Fast graph analysis engine";
  m.def("pagerank",
        [](const std::string &file_name, const u32 num_iters) {
          return pagerank(file_name, num_iters);
        },
        py::arg("file_name"), py::arg("num_iters") = 50);
  m.def("clustering",
        [](const std::string &file_name, const u32 num_clusters_hint,
           const f64 threshold) {
          return clustering(file_name, num_clusters_hint, threshold);
        },
        py::arg("file_name"), py::arg("num_clusters_hint") = 100,
        py::arg("threshold") = 0.00003);
}
} // namespace hoshizora