#pragma once

#include <cmath>
#include <algorithm>
#include <qf/core/types.hpp>

namespace qf {

struct Tolerance {
    Real absolute = 1e-12;
    Real relative = 1e-10;
};

[[nodiscard]] constexpr bool valid_tolerance(Tolerance t) noexcept {
    return t.absolute >= 0.0 && t.relative >= 0.0 && std::isfinite(t.absolute) && std::isfinite(t.relative);
}

[[nodiscard]] constexpr bool is_finite(Real x) noexcept {
    return std::isfinite(x);
}

[[nodiscard]] constexpr bool is_nan(Real x) noexcept {
    return std::isnan(x);
}

[[nodiscard]] constexpr bool is_infinite(Real x) noexcept {
    return std::isinf(x);
}

[[nodiscard]] constexpr bool almost_equal(Real a, Real b, Tolerance tolerance = {}) noexcept {
    if (a == b) {
        return true;
    }
    if (!std::isfinite(a) || !std::isfinite(b)) {
        return false;
    }
    const Real scale = std::max(std::abs(a), std::abs(b));
    return std::abs(a - b) <= std::max(tolerance.absolute, tolerance.relative * scale);
}

[[nodiscard]] constexpr Real clamp(Real x, Real lo, Real hi) noexcept {
    return x < lo ? lo : (x > hi ? hi : x);
}

} // namespace qf
