#pragma once

#include <limits>
#include <qf/core/types.hpp>

namespace qf::constants {

inline constexpr Real pi = 3.141592653589793238462643383279502884;
inline constexpr Real two_pi = 2.0 * pi;
inline constexpr Real half_pi = 0.5 * pi;
inline constexpr Real e = 2.718281828459045235360287471352662498;
inline constexpr Real sqrt_two = 1.414213562373095048801688724209698079;
inline constexpr Real sqrt_two_pi = 2.506628274631000502415765284811045253;
inline constexpr Real machine_epsilon = std::numeric_limits<Real>::epsilon();
inline constexpr Real infinity = std::numeric_limits<Real>::infinity();
inline constexpr Real quiet_nan = std::numeric_limits<Real>::quiet_NaN();

} // namespace qf::constants
