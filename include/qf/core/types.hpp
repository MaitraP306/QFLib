#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace qf {

using Real = double;
using Index = std::size_t;
using SignedIndex = std::ptrdiff_t;
using Count = std::size_t;
using TimestampRep = std::int64_t;

inline constexpr Index npos = std::numeric_limits<Index>::max();

} // namespace qf
