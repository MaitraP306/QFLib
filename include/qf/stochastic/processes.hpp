#pragma once

#include <cmath>

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/math/scalar.hpp>

namespace qf::stochastic {

struct BrownianMotion final {
    [[nodiscard]] static Result<Real> increment(Real dt, Real standard_normal) noexcept;
    [[nodiscard]] static Real increment_unchecked(Real dt, Real standard_normal) noexcept {
        return std::sqrt(dt) * standard_normal;
    }
};

struct GBMParameters final {
    Real mu{};
    Real sigma{};
};

[[nodiscard]] Result<Real> gbm_step(Real spot, const GBMParameters& parameters, Real dt, Real standard_normal) noexcept;
[[nodiscard]] Real gbm_step_unchecked(Real spot, const GBMParameters& parameters, Real dt, Real standard_normal) noexcept;


[[nodiscard]] Result<Real> ou_step(Real value, Real theta, Real mu, Real sigma, Real dt, Real standard_normal) noexcept;
[[nodiscard]] Real ou_step_unchecked(Real value, Real theta, Real mu, Real sigma, Real dt, Real standard_normal) noexcept;

struct VasicekParameters final {
    Real kappa{};
    Real theta{};
    Real sigma{};
};

[[nodiscard]] Result<Real> vasicek_step(Real rate, const VasicekParameters& parameters, Real dt, Real standard_normal) noexcept;
[[nodiscard]] Real vasicek_step_unchecked(Real rate, const VasicekParameters& parameters, Real dt, Real standard_normal) noexcept;

struct CIRParameters final {
    Real kappa{};
    Real theta{};
    Real sigma{};
};

[[nodiscard]] Result<Real> cir_step(Real rate, const CIRParameters& parameters, Real dt, Real standard_normal) noexcept;
[[nodiscard]] Real cir_step_unchecked(Real rate, const CIRParameters& parameters, Real dt, Real standard_normal) noexcept;


template <typename Rng>
[[nodiscard]] Status simulate_gbm_into_unchecked(Real initial, const GBMParameters& parameters, Real dt, Rng& rng, Span<Real> output) noexcept {
    if (output.empty())
        return {ErrorCode::invalid_argument, "GBM output must not be empty"};
    output[0] = initial;
    Real state = initial;
    const Real drift = (parameters.mu - 0.5 * parameters.sigma * parameters.sigma) * dt;
    const Real diffusion = parameters.sigma * std::sqrt(dt);
    for (Index i = 1; i < output.size(); ++i) {
        state *= std::exp(drift + diffusion * rng.normal());
        output[i] = state;
    }
    return {};
}

template <typename Rng>
[[nodiscard]] Status simulate_ou_into_unchecked(Real initial, Real theta, Real mu, Real sigma, Real dt, Rng& rng, Span<Real> output) noexcept {
    if (output.empty())
        return {ErrorCode::invalid_argument, "OU output must not be empty"};
    output[0] = initial;
    const Real phi = std::exp(-theta * dt);
    const Real innovation_scale = sigma * std::sqrt(-std::expm1(-2.0 * theta * dt) / (2.0 * theta));
    Real state = initial;
    for (Index i = 1; i < output.size(); ++i) {
        state = mu + (state - mu) * phi + innovation_scale * rng.normal();
        output[i] = state;
    }
    return {};
}

} // namespace qf::stochastic
