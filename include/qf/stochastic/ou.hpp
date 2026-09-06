#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/time.hpp>
#include <qf/core/types.hpp>
#include <qf/timeseries/series.hpp>

namespace qf::stochastic {

// Parameters of the continuous-time Ornstein-Uhlenbeck process
//
//     dX_t = theta (mu - X_t) dt + sigma dW_t,  theta > 0.
//
// phi is the exact discrete-time AR(1) coefficient over dt:
//     phi = exp(-theta * dt).
struct OUParameters final {
    Real theta{};       // Mean-reversion speed.
    Real mu{};          // Long-run equilibrium level.
    Real sigma{};       // Instantaneous diffusion coefficient.
    Real phi{};         // Discrete-time persistence over dt.
    Real half_life{};   // log(2) / theta, in the same time unit as dt.
    Real dt{};          // Sampling interval in seconds.
};

// Estimate OU parameters from equally spaced observations using the exact
// discrete-time transition and OLS/conditional maximum-likelihood estimator.
// No design matrix is materialized; the regression sufficient statistics are
// accumulated directly in one pass.
[[nodiscard]] Result<OUParameters> estimate_ou(ConstSpan<Real> values,
                                                Duration dt) noexcept;

// Convenience overload for a TimeSeries. Timestamps must be strictly
// increasing and exactly equally spaced.
[[nodiscard]] Result<OUParameters> estimate_ou(const timeseries::TimeSeries& series) noexcept;

} // namespace qf::stochastic
