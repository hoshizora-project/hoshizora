#ifndef COMPRESS_EXTERNAL
#define COMPRESS_EXTERNAL

#include "external/FastPFor/headers/codecfactory.h"

using namespace std;

/*
 * external codec
 */
const auto pfor = new FastPForLib::CompositeCodec<
  FastPForLib::SIMDFastPFor<8>, FastPForLib::VariableByte>();

constexpr auto THRESHOLD = 64ul; // TODO

#endif // COMPRESS_EXTERNAL
