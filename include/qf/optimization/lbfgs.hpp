#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <limits>

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::optimization {

struct LbfgsOptions final {
    Count max_iterations = 100;
    Count history_size = 8;
    Real gradient_tolerance = 1e-8;
    Real step_tolerance = 1e-12;
    Real initial_step = 1.0;
    Real armijo = 1e-4;
    Real backtrack = 0.5;
    Count max_line_search_iterations = 24;
};

struct LbfgsWorkspace final {
    Span<Real> s_history{};
    Span<Real> y_history{};
    Span<Real> rho{};
    Span<Real> alpha{};
    Span<Real> gradient{};
    Span<Real> next_gradient{};
    Span<Real> direction{};

    [[nodiscard]] bool valid(Count n, Count m) const noexcept {
        return n > 0 && m > 0 && s_history.size() >= n * m && y_history.size() >= n * m &&
               rho.size() >= m && alpha.size() >= m && gradient.size() >= n &&
               next_gradient.size() >= n && direction.size() >= n;
    }
};

struct OptimizeResult final {
    Real objective{};
    Real gradient_norm{};
    Count iterations{};
    bool converged{};
};

namespace detail {
template <typename F>
concept ObjectiveWithGradient = requires(F&& f, ConstSpan<Real> x, Span<Real> g) {
    { f(x, g) } -> std::convertible_to<Real>;
};

[[nodiscard]] inline Real norm2(ConstSpan<Real> x) noexcept {
    Real s = 0.0;
    for (Real v : x) s += v * v;
    return std::sqrt(s);
}
}

template <detail::ObjectiveWithGradient F>
[[nodiscard]] Result<OptimizeResult> lbfgs(
    Span<Real> x, F&& objective_gradient, LbfgsWorkspace workspace,
    LbfgsOptions o = {}) noexcept {
    const Count n = x.size();
    const Count m = o.history_size;
    if (!workspace.valid(n, m) || o.max_iterations == 0 || o.history_size == 0 ||
        !(o.gradient_tolerance >= 0.0) || !(o.step_tolerance >= 0.0) ||
        !(o.initial_step > 0.0) || !(o.armijo > 0.0 && o.armijo < 1.0) ||
        !(o.backtrack > 0.0 && o.backtrack < 1.0))
        return Status{ErrorCode::invalid_argument, "invalid L-BFGS workspace or options"};

    Real fx = static_cast<Real>(objective_gradient(ConstSpan<Real>{x.data(), n}, workspace.gradient));
    if (!std::isfinite(fx)) return Status{ErrorCode::non_finite_input, "objective returned non-finite value"};
    Real gnorm = detail::norm2(ConstSpan<Real>{workspace.gradient.data(), n});
    if (!std::isfinite(gnorm)) return Status{ErrorCode::non_finite_input, "gradient returned non-finite value"};
    if (gnorm <= o.gradient_tolerance) return OptimizeResult{fx, gnorm, 0, true};

    Count history_count = 0;
    Count history_head = 0;
    for (Count iter = 0; iter < o.max_iterations; ++iter) {
        for (Index i = 0; i < n; ++i) workspace.direction[i] = workspace.gradient[i];
        for (Count r = 0; r < history_count; ++r) {
            const Count idx = (history_head + m - 1 - r) % m;
            const Real* s = workspace.s_history.data() + idx * n;
            const Real* y = workspace.y_history.data() + idx * n;
            Real dot = 0.0;
            for (Index i = 0; i < n; ++i) dot += s[i] * workspace.direction[i];
            workspace.alpha[r] = workspace.rho[idx] * dot;
            for (Index i = 0; i < n; ++i) workspace.direction[i] -= workspace.alpha[r] * y[i];
        }
        Real gamma = 1.0;
        if (history_count != 0) {
            const Count idx = (history_head + m - 1) % m;
            const Real* s = workspace.s_history.data() + idx * n;
            const Real* y = workspace.y_history.data() + idx * n;
            Real sy = 0.0, yy = 0.0;
            for (Index i = 0; i < n; ++i) { sy += s[i] * y[i]; yy += y[i] * y[i]; }
            if (yy > 0.0 && std::isfinite(sy / yy)) gamma = sy / yy;
        }
        for (Index i = 0; i < n; ++i) workspace.direction[i] *= gamma;
        for (Count r = history_count; r-- > 0;) {
            const Count idx = (history_head + m - history_count + r) % m;
            const Real* s = workspace.s_history.data() + idx * n;
            const Real* y = workspace.y_history.data() + idx * n;
            Real dot = 0.0;
            for (Index i = 0; i < n; ++i) dot += y[i] * workspace.direction[i];
            const Real beta = workspace.rho[idx] * dot;
            for (Index i = 0; i < n; ++i) workspace.direction[i] += s[i] * (workspace.alpha[history_count - 1 - r] - beta);
        }
        Real directional = 0.0;
        for (Index i = 0; i < n; ++i) {
            workspace.direction[i] = -workspace.direction[i];
            directional += workspace.gradient[i] * workspace.direction[i];
        }
        if (!(directional < 0.0) || !std::isfinite(directional)) {
            for (Index i = 0; i < n; ++i) workspace.direction[i] = -workspace.gradient[i];
            directional = -gnorm * gnorm;
        }

        const Real* old_x = x.data();
        const Count slot = history_head;
        Real* s = workspace.s_history.data() + slot * n;
        for (Index i = 0; i < n; ++i) s[i] = x[i];

        Real step = o.initial_step;
        Real accepted_fx = std::numeric_limits<Real>::infinity();
        bool accepted = false;
        for (Count ls = 0; ls < o.max_line_search_iterations; ++ls) {
            for (Index i = 0; i < n; ++i) x[i] = s[i] + step * workspace.direction[i];
            accepted_fx = static_cast<Real>(objective_gradient(ConstSpan<Real>{x.data(), n}, workspace.next_gradient));
            if (std::isfinite(accepted_fx) && accepted_fx <= fx + o.armijo * step * directional) {
                accepted = true;
                break;
            }
            step *= o.backtrack;
            if (step <= o.step_tolerance) break;
        }
        if (!accepted) {
            for (Index i = 0; i < n; ++i) x[i] = s[i];
            return OptimizeResult{fx, gnorm, iter, false};
        }

        Real* y = workspace.y_history.data() + slot * n;
        Real sy = 0.0;
        for (Index i = 0; i < n; ++i) {
            const Real dx = x[i] - s[i];
            y[i] = workspace.next_gradient[i] - workspace.gradient[i];
            sy += dx * y[i];
            s[i] = dx; 
        }
        if (sy > 1e-16) {
            workspace.rho[slot] = 1.0 / sy;
            history_head = (history_head + 1) % m;
            history_count = std::min(history_count + 1, m);
        }
        for (Index i = 0; i < n; ++i) workspace.gradient[i] = workspace.next_gradient[i];
        fx = accepted_fx;
        gnorm = detail::norm2(ConstSpan<Real>{workspace.gradient.data(), n});
        if (!std::isfinite(gnorm)) return Status{ErrorCode::numerical_failure, "gradient became non-finite"};
        if (gnorm <= o.gradient_tolerance || step * detail::norm2(ConstSpan<Real>{workspace.direction.data(), n}) <= o.step_tolerance)
            return OptimizeResult{fx, gnorm, iter + 1, true};
        (void)old_x;
    }
    return OptimizeResult{fx, gnorm, o.max_iterations, false};
}

} // namespace qf::optimization
