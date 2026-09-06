#pragma once

#include <limits>

namespace qf::constants {

inline constexpr double pi = 3.141592653589793238462643383279502884;
inline constexpr double two_pi = 2.0 * pi;
inline constexpr double half_pi = 0.5 * pi;
inline constexpr double e = 2.718281828459045235360287471352662498;
inline constexpr double sqrt_two = 1.414213562373095048801688724209698079;
inline constexpr double sqrt_two_pi = 2.506628274631000502415765284811045253;
inline constexpr double machine_epsilon = std::numeric_limits<double>::epsilon();
inline constexpr double infinity = std::numeric_limits<double>::infinity();
inline constexpr double quiet_nan = std::numeric_limits<double>::quiet_NaN();

} // namespace qf::constants
