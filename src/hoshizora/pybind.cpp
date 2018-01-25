#include <pybind11/pybind11.h>

namespace hoshizora {
PYBIND11_MODULE(hoshizora, m) {
  namespace py = pybind11;

  m.doc() = "hoshizora: Fast graph analysis engine";
}
} // namespace hoshizora