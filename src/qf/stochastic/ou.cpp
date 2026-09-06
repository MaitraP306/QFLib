#include <qf/stochastic/ou.hpp>

#include <cmath>
#include <limits>

namespace qf::stochastic {

namespace {

struct RegressionSufficientStatistics final {
    Count count{};
    Real mean_x{};
    Real mean_y{};
    Real sxx{};
    Real sxy{};
};

Result<RegressionSufficientStatistics> accumulate_ar1(ConstSpan<Real> values) noexcept {
    if (values.size() < 3)
        return Status{ErrorCode::invalid_argument, "OU estimation requires at least three observations"};

    RegressionSufficientStatistics s{};
    // Online bivariate covariance update for pairs
    // (x_t, y_t) = (X_{t-1}, X_t). This avoids constructing two temporary
    // vectors or an N x 2 design matrix.
    for (Index i = 1; i < values.size(); ++i) {
        const Real x = values[i - 1];
        const Real y = values[i];
        if (!std::isfinite(x) || !std::isfinite(y))
            return Status{ErrorCode::non_finite_input, "OU estimation received a non-finite observation"};

        ++s.count;
        const Real dx = x - s.mean_x;
        const Real dy = y - s.mean_y;
        const Real inv_n = 1.0 / static_cast<Real>(s.count);
        s.mean_x += dx * inv_n;
        s.mean_y += dy * inv_n;
        // The centered cross-product update is numerically stable and O(1).
        s.sxx += dx * (x - s.mean_x);
        s.sxy += dx * (y - s.mean_y);
    }
    return s;
}

} // namespace

Result<OUParameters> estimate_ou(ConstSpan<Real> values, Duration dt) noexcept {
    const Real dt_seconds = dt.seconds();
    if (!(dt_seconds > 0.0) || !std::isfinite(dt_seconds))
        return Status{ErrorCode::invalid_argument, "OU sampling interval must be finite and positive"};

    auto stats_result = accumulate_ar1(values);
    if (!stats_result) return stats_result.error();
    const auto& s = stats_result.value();

    if (!(s.sxx > 0.0) || !std::isfinite(s.sxx))
        return Status{ErrorCode::singular, "OU regression has zero or non-finite predictor variance"};

    // y = alpha + phi*x. For an exact OU transition:
    // alpha = mu * (1 - phi), phi = exp(-theta*dt).
    const Real phi = s.sxy / s.sxx;
    if (!(phi > 0.0 && phi < 1.0) || !std::isfinite(phi))
        return Status{ErrorCode::numerical_failure,
                      "OU estimate requires a stationary mean-reverting persistence 0 < phi < 1"};

    const Real alpha = s.mean_y - phi * s.mean_x;
    // Compute the OU conversion with expm1 to avoid cancellation when phi is
    // close to one (the common case for slowly mean-reverting financial series).
    const Real log_phi = std::log(phi);
    const Real one_minus_phi = -std::expm1(log_phi);
    if (!(one_minus_phi > 0.0) || !std::isfinite(one_minus_phi))
        return Status{ErrorCode::numerical_failure, "OU persistence is too close to one"};

    const Real mu = alpha / one_minus_phi;
    // -log(phi) is the exact continuous-time mean-reversion rate over dt.
    const Real theta = -log_phi / dt_seconds;
    const Real half_life = std::log(2.0) / theta;

    if (!(theta > 0.0) || !std::isfinite(theta) || !std::isfinite(mu) || !std::isfinite(half_life))
        return Status{ErrorCode::numerical_failure, "OU parameter conversion failed"};

    // Conditional Gaussian MLE for the exact OU transition:
    // Var[e_t] = sigma^2 * (1 - phi^2) / (2*theta).
    // Use SSE / N for the conditional MLE.
    Real sse = 0.0;
    Real compensation = 0.0;
    for (Index i = 1; i < values.size(); ++i) {
        const Real residual = values[i] - alpha - phi * values[i - 1];
        const Real term = residual * residual;
        const Real corrected = term - compensation;
        const Real updated = sse + corrected;
        compensation = (updated - sse) - corrected;
        sse = updated;
    }
    if (!std::isfinite(sse))
        return Status{ErrorCode::numerical_failure, "OU residual sum of squares is non-finite"};

    const Real n = static_cast<Real>(s.count);
    const Real variance_factor = -std::expm1(2.0 * log_phi) / (2.0 * theta);
    if (!(variance_factor > 0.0) || !std::isfinite(variance_factor))
        return Status{ErrorCode::numerical_failure, "OU transition variance is invalid"};

    const Real innovation_variance = sse / n;
    const Real sigma_squared = innovation_variance / variance_factor;
    if (sigma_squared < 0.0 || !std::isfinite(sigma_squared))
        return Status{ErrorCode::numerical_failure, "OU diffusion estimate is invalid"};

    return OUParameters{theta, mu, std::sqrt(sigma_squared), phi, half_life, dt_seconds};
}

Result<OUParameters> estimate_ou(const timeseries::TimeSeries& series) noexcept {
    if (series.size() < 3)
        return Status{ErrorCode::invalid_argument, "OU estimation requires at least three observations"};

    const auto timestamps = series.timestamps();
    const auto values = series.values();
    const auto first_dt = timestamps[1] - timestamps[0];
    if (first_dt.count() <= 0)
        return Status{ErrorCode::invalid_argument, "OU timestamps must be strictly increasing"};

    for (Index i = 2; i < timestamps.size(); ++i) {
        if (timestamps[i] <= timestamps[i - 1])
            return Status{ErrorCode::invalid_argument, "OU timestamps must be strictly increasing"};
        if ((timestamps[i] - timestamps[i - 1]).count() != first_dt.count())
            return Status{ErrorCode::invalid_argument, "OU estimation requires equally spaced timestamps"};
    }

    return estimate_ou(values, first_dt);
}

} // namespace qf::stochastic
