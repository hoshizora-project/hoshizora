#include "hoshizora/app/apps.h"
#include <iostream>
#include <map>
#include <set>
#include <utility>

namespace hoshizora {
void main(int argc, char *argv[]) {
  const auto type = std::string(argv[1]);
  const auto file_name = std::string(argv[2]);

  init();

  if (type == "pagerank") {
    const auto num_iters = (u32)std::strtol(argv[3], nullptr, 10);
    const auto res = pagerank(file_name, num_iters);
    for (const auto &el : res) {
      printf("%s\n", el.c_str());
    }
  } else if (type == "clustering") {
    const auto num_clusters_hint = (u32)std::strtol(argv[3], nullptr, 10);
    const auto threshold = argc > 4 ? std::stof(argv[4]) : 0.00003;
    auto res = clustering(argv[2], num_clusters_hint, threshold);
    for (const auto &el : res) {
      printf("%d\n", el);
    }
  } else {
    printf("'%s' is not specified\n", type.c_str());
  }
}
} // namespace hoshizora

int main(int argc, char *argv[]) {
  hoshizora::main(argc, argv);
  return 0;
}
