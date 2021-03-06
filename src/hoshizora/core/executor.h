#ifndef HOSHIZORA_EXECUTOR_H
#define HOSHIZORA_EXECUTOR_H

#include "hoshizora/core/includes.h"
#include <string>

namespace hoshizora {
template <class Kernel> class Executor {
  // template<class Result>
  virtual std::vector<std::string> run() = 0;
};
} // namespace hoshizora

#endif // HOSHIZORA_EXECUTOR_H
