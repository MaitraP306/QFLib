#pragma once

#include <cmath>
#include <limits>
#include <type_traits>

namespace qf {

struct Tolerance {
    double absolute = 1e-12;
    double relative = 1e-10;
};

[[nodiscard]] constexpr bool valid_tolerance(Tolerance t) noexcept {
    return t.absolute >= 0.0 && t.relative >= 0.0 &&
           std::isfinite(t.absolute) && std::isfinite(t.relative);
}

[[nodiscard]] constexpr bool is_finite(double x) noexcept {
    return std::isfinite(x);
}

[[nodiscard]] constexpr bool is_nan(double x) noexcept {
    return std::isnan(x);
}

[[nodiscard]] constexpr bool is_infinite(double x) noexcept {
    return std::isinf(x);
}

[[nodiscard]] constexpr bool almost_equal(double a, double b,
                                           Tolerance tolerance = {}) noexcept {
    if (a == b) {
        return true;
    }
    if (!std::isfinite(a) || !std::isfinite(b)) {
        return false;
    }
    const double scale = std::max(std::abs(a), std::abs(b));
    return std::abs(a - b) <=
           std::max(tolerance.absolute, tolerance.relative * scale);
}

[[nodiscard]] constexpr double clamp(double x, double lo, double hi) noexcept {
    return x < lo ? lo : (x > hi ? hi : x);
}

} // namespace qf
