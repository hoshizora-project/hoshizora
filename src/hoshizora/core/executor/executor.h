#ifndef HOSHIZORA_EXECUTOR_H
#define HOSHIZORA_EXECUTOR_H

#include "hoshizora/core/util/includes.h"
#include <string>

namespace hoshizora {
template <class Kernel> class Executor {
  // template<class Result>
  virtual std::string run() = 0;
};
} // namespace hoshizora

#endif // HOSHIZORA_EXECUTOR_H
