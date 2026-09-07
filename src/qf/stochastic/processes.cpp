#include <qf/stochastic/processes.hpp>

#include <algorithm>
#include <cmath>

namespace qf::stochastic {
namespace {

[[nodiscard]] bool positive_finite(Real x) noexcept {
    return std::isfinite(x) && x > 0.0;
}

[[nodiscard]] bool finite(Real x) noexcept { return std::isfinite(x); }

} // namespace

Result<Real> BrownianMotion::increment(Real dt, Real standard_normal) noexcept {
    if (!positive_finite(dt) || !finite(standard_normal))
        return Status{ErrorCode::invalid_argument, "Brownian increment requires finite dt > 0 and finite normal innovation"};
    return std::sqrt(dt) * standard_normal;
}

Real gbm_step_unchecked(Real spot, const GBMParameters& p, Real dt, Real z) noexcept {
    const Real variance = p.sigma * p.sigma;
    return spot * std::exp((p.mu - 0.5 * variance) * dt + p.sigma * std::sqrt(dt) * z);
}

Result<Real> gbm_step(Real spot, const GBMParameters& p, Real dt, Real z) noexcept {
    if (!positive_finite(spot) || !finite(p.mu) || !std::isfinite(p.sigma) || p.sigma < 0.0 ||
        !positive_finite(dt) || !finite(z))
        return Status{ErrorCode::invalid_argument, "GBM parameters or state are invalid"};
    const Real next = gbm_step_unchecked(spot, p, dt, z);
    if (!finite(next) || !(next > 0.0))
        return Status{ErrorCode::numerical_failure, "GBM transition is non-finite"};
    return next;
}

Real ou_step_unchecked(Real value, Real theta, Real mu, Real sigma, Real dt, Real z) noexcept {
    const Real phi = std::exp(-theta * dt);
    const Real variance_factor = -std::expm1(-2.0 * theta * dt) / (2.0 * theta);
    return mu + (value - mu) * phi + sigma * std::sqrt(variance_factor) * z;
}

Result<Real> ou_step(Real value, Real theta, Real mu, Real sigma, Real dt, Real z) noexcept {
    if (!finite(value) || !positive_finite(theta) || !finite(mu) || !finite(sigma) || sigma < 0.0 ||
        !positive_finite(dt) || !finite(z))
        return Status{ErrorCode::invalid_argument, "OU transition parameters are invalid"};
    const Real next = ou_step_unchecked(value, theta, mu, sigma, dt, z);
    if (!finite(next))
        return Status{ErrorCode::numerical_failure, "OU transition is non-finite"};
    return next;
}

Real vasicek_step_unchecked(Real rate, const VasicekParameters& p, Real dt, Real z) noexcept {
    const Real phi = std::exp(-p.kappa * dt);
    const Real variance_factor = -std::expm1(-2.0 * p.kappa * dt) / (2.0 * p.kappa);
    return p.theta + (rate - p.theta) * phi + p.sigma * std::sqrt(variance_factor) * z;
}

Result<Real> vasicek_step(Real rate, const VasicekParameters& p, Real dt, Real z) noexcept {
    if (!finite(rate) || !positive_finite(p.kappa) || !finite(p.theta) || !finite(p.sigma) || p.sigma < 0.0 ||
        !positive_finite(dt) || !finite(z))
        return Status{ErrorCode::invalid_argument, "Vasicek transition parameters are invalid"};
    const Real next = vasicek_step_unchecked(rate, p, dt, z);
    if (!finite(next))
        return Status{ErrorCode::numerical_failure, "Vasicek transition is non-finite"};
    return next;
}

Real cir_step_unchecked(Real rate, const CIRParameters& p, Real dt, Real z) noexcept {
    const Real positive_rate = std::max(rate, 0.0);
    return std::max(0.0,
        rate + p.kappa * (p.theta - positive_rate) * dt + p.sigma * std::sqrt(positive_rate * dt) * z);
}

Result<Real> cir_step(Real rate, const CIRParameters& p, Real dt, Real z) noexcept {
    if (!finite(rate) || rate < 0.0 || !positive_finite(p.kappa) || !positive_finite(p.theta) ||
        !finite(p.sigma) || p.sigma < 0.0 || !positive_finite(dt) || !finite(z))
        return Status{ErrorCode::invalid_argument, "CIR transition parameters are invalid"};
    const Real next = cir_step_unchecked(rate, p, dt, z);
    if (!finite(next))
        return Status{ErrorCode::numerical_failure, "CIR transition is non-finite"};
    return next;
}

} // namespace qf::stochastic
