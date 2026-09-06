#pragma once

#include <cmath>
#include <type_traits>

namespace qf::math {

template <typename T>
[[nodiscard]] constexpr T square(T x) noexcept { return x * x; }

template <typename T>
[[nodiscard]] constexpr T cube(T x) noexcept { return x * x * x; }

template <typename T>
[[nodiscard]] constexpr int sign(T x) noexcept { return (T{0} < x) - (x < T{0}); }

template <typename T>
[[nodiscard]] constexpr T lerp(T a, T b, T t) noexcept { return a + t * (b - a); }

[[nodiscard]] double hypot2(double x, double y) noexcept;
[[nodiscard]] double exp(double x) noexcept;
[[nodiscard]] double expm1(double x) noexcept;
[[nodiscard]] double log(double x) noexcept;
[[nodiscard]] double log1p(double x) noexcept;
[[nodiscard]] double sqrt(double x) noexcept;
[[nodiscard]] double cbrt(double x) noexcept;
[[nodiscard]] double sin(double x) noexcept;
[[nodiscard]] double cos(double x) noexcept;
[[nodiscard]] double tan(double x) noexcept;
[[nodiscard]] double asin(double x) noexcept;
[[nodiscard]] double acos(double x) noexcept;
[[nodiscard]] double atan(double x) noexcept;
[[nodiscard]] double atan2(double y, double x) noexcept;
[[nodiscard]] double floor(double x) noexcept;
[[nodiscard]] double ceil(double x) noexcept;
[[nodiscard]] double round(double x) noexcept;
[[nodiscard]] double log1pexp(double x) noexcept;
[[nodiscard]] double sigmoid(double x) noexcept;
[[nodiscard]] double logit(double p) noexcept;

} // namespace qf::math
