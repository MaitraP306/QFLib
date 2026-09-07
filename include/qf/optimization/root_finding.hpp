#pragma once

#include <cmath>
#include <concepts>
#include <limits>

#include <qf/core/error.hpp>
#include <qf/core/numerical.hpp>
#include <qf/core/types.hpp>

namespace qf::optimization {

struct RootResult final {
    Real root{};
    Real function_value{};
    Count iterations{};
    bool converged{};
};

struct RootOptions final {
    Count max_iterations = 100;
    Real absolute_tolerance = 1e-12;
    Real relative_tolerance = 1e-10;
};

namespace detail {
template <typename F>
concept ScalarFunction = requires(F&& f, Real x) {
    { f(x) } -> std::convertible_to<Real>;
};

[[nodiscard]] constexpr bool root_options_valid(const RootOptions& o) noexcept {
    return o.max_iterations > 0 && std::isfinite(o.absolute_tolerance) &&
           std::isfinite(o.relative_tolerance) && o.absolute_tolerance >= 0.0 &&
           o.relative_tolerance >= 0.0;
}

[[nodiscard]] constexpr Real root_tol(Real x, const RootOptions& o) noexcept {
    return std::max(o.absolute_tolerance, o.relative_tolerance * std::abs(x));
}
} // namespace detail

template <detail::ScalarFunction F>
[[nodiscard]] Result<RootResult> bisection(F&& f, Real lo, Real hi, RootOptions o = {}) noexcept {
    if (!detail::root_options_valid(o) || !std::isfinite(lo) || !std::isfinite(hi) || !(lo < hi))
        return Status{ErrorCode::invalid_argument, "invalid bisection interval or options"};
    const Real flo = static_cast<Real>(f(lo));
    const Real fhi = static_cast<Real>(f(hi));
    if (!std::isfinite(flo) || !std::isfinite(fhi))
        return Status{ErrorCode::non_finite_input, "root function returned non-finite value"};
    if (flo == 0.0) return RootResult{lo, flo, 0, true};
    if (fhi == 0.0) return RootResult{hi, fhi, 0, true};
    if ((flo > 0.0) == (fhi > 0.0))
        return Status{ErrorCode::invalid_argument, "bisection interval must bracket a root"};

    Real a = lo, b = hi, fa = flo, fb = fhi;
    for (Count iter = 1; iter <= o.max_iterations; ++iter) {
        const Real mid = a + 0.5 * (b - a);
        const Real fm = static_cast<Real>(f(mid));
        if (!std::isfinite(fm)) return Status{ErrorCode::numerical_failure, "root function returned non-finite value"};
        if (fm == 0.0 || std::abs(b - a) <= detail::root_tol(mid, o))
            return RootResult{mid, fm, iter, true};
        if ((fa > 0.0) == (fm > 0.0)) { a = mid; fa = fm; }
        else { b = mid; fb = fm; }
        (void)fb;
    }
    const Real root = a + 0.5 * (b - a);
    return RootResult{root, static_cast<Real>(f(root)), o.max_iterations, false};
}

template <detail::ScalarFunction F>
[[nodiscard]] Result<RootResult> brent(F&& f, Real lo, Real hi, RootOptions o = {}) noexcept {
    if (!detail::root_options_valid(o) || !std::isfinite(lo) || !std::isfinite(hi) || !(lo < hi))
        return Status{ErrorCode::invalid_argument, "invalid Brent interval or options"};
    Real a = lo, b = hi;
    Real fa = static_cast<Real>(f(a)), fb = static_cast<Real>(f(b));
    if (!std::isfinite(fa) || !std::isfinite(fb))
        return Status{ErrorCode::non_finite_input, "root function returned non-finite value"};
    if (fa == 0.0) return RootResult{a, fa, 0, true};
    if (fb == 0.0) return RootResult{b, fb, 0, true};
    if ((fa > 0.0) == (fb > 0.0))
        return Status{ErrorCode::invalid_argument, "Brent interval must bracket a root"};

    Real c = b, fc = fb;
    Real d = b - a, e = d;
    for (Count iter = 1; iter <= o.max_iterations; ++iter) {
        if ((fb > 0.0) == (fc > 0.0)) { c = a; fc = fa; d = b - a; e = d; }
        if (std::abs(fc) < std::abs(fb)) {
            const Real old_a = a, old_fa = fa;
            a = b; fa = fb;
            b = c; fb = fc;
            c = old_a; fc = old_fa;
        }
        const Real tol = detail::root_tol(b, o);
        const Real m = 0.5 * (c - b);
        if (std::abs(m) <= tol || fb == 0.0)
            return RootResult{b, fb, iter, true};

        if (std::abs(e) >= tol && std::abs(fa) > std::abs(fb)) {
            const Real s = fb / fa;
            Real p, q;
            if (a == c) {
                p = 2.0 * m * s;
                q = 1.0 - s;
            } else {
                const Real q1 = fa / fc;
                const Real r = fb / fc;
                p = s * (2.0 * m * q1 * (q1 - r) - (b - a) * (r - 1.0));
                q = (q1 - 1.0) * (r - 1.0) * (s - 1.0);
            }
            if (p > 0.0) q = -q; else p = -p;
            const Real min1 = 3.0 * m * q - std::abs(tol * q);
            const Real min2 = std::abs(e * q);
            if (2.0 * p < std::min(min1, min2)) { e = d; d = p / q; }
            else { d = m; e = m; }
        } else { d = m; e = m; }

        a = b; fa = fb;
        b += (std::abs(d) > tol) ? d : (m > 0.0 ? tol : -tol);
        fb = static_cast<Real>(f(b));
        if (!std::isfinite(fb)) return Status{ErrorCode::numerical_failure, "root function returned non-finite value"};
    }
    return RootResult{b, fb, o.max_iterations, false};
}

template <typename F, typename DF>
requires requires(F&& f, DF&& df, Real x) {
    { f(x) } -> std::convertible_to<Real>;
    { df(x) } -> std::convertible_to<Real>;
}
[[nodiscard]] Result<RootResult> newton(F&& f, DF&& df, Real initial, RootOptions o = {}, Real bracket_lo = -std::numeric_limits<Real>::infinity(), Real bracket_hi = std::numeric_limits<Real>::infinity()) noexcept {
    if (!detail::root_options_valid(o) || !std::isfinite(initial) ||
        (std::isfinite(bracket_lo) && std::isfinite(bracket_hi) && !(bracket_lo < bracket_hi)))
        return Status{ErrorCode::invalid_argument, "invalid Newton initial value or options"};
    Real x = initial;
    for (Count iter = 1; iter <= o.max_iterations; ++iter) {
        const Real fx = static_cast<Real>(f(x));
        const Real dfx = static_cast<Real>(df(x));
        if (!std::isfinite(fx) || !std::isfinite(dfx))
            return Status{ErrorCode::numerical_failure, "Newton function returned non-finite value"};
        if (std::abs(fx) <= o.absolute_tolerance + o.relative_tolerance * std::abs(x))
            return RootResult{x, fx, iter, true};
        if (dfx == 0.0) return Status{ErrorCode::singular, "Newton derivative is zero"};
        Real step = fx / dfx;
        Real next = x - step;
        if (std::isfinite(bracket_lo) && next < bracket_lo) next = 0.5 * (x + bracket_lo);
        if (std::isfinite(bracket_hi) && next > bracket_hi) next = 0.5 * (x + bracket_hi);
        if (!std::isfinite(next)) return Status{ErrorCode::numerical_failure, "Newton step became non-finite"};
        if (std::abs(next - x) <= detail::root_tol(next, o)) {
            const Real fn = static_cast<Real>(f(next));
            if (std::isfinite(fn) && std::abs(fn) <= o.absolute_tolerance + o.relative_tolerance * std::abs(next))
                return RootResult{next, fn, iter, true};
        }
        x = next;
    }
    return RootResult{x, static_cast<Real>(f(x)), o.max_iterations, false};
}

} // namespace qf::optimization
