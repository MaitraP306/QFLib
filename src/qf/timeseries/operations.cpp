#include <qf/timeseries/operations.hpp>

#include <cmath>
#include <limits>

#include <qf/core/numerical.hpp>
#include <qf/math/scalar.hpp>

namespace qf::timeseries {
namespace {

Result<linalg::Vector<Real>> validate_lagged_input(ConstSpan<Real> values, Count lag) noexcept {
    if (lag == 0 || lag >= values.size()) {
        return Status{ErrorCode::invalid_argument, "lag must be positive and smaller than the number of observations"};
    }
    for (Real x : values) {
        if (!std::isfinite(x)) return Status{ErrorCode::non_finite_input, "time-series operation received a non-finite observation"};
    }
    return linalg::Vector<Real>(values.size() - lag);
}

} // namespace

Result<linalg::Vector<Real>> difference(ConstSpan<Real> values, Count lag) noexcept {
    auto out_result = validate_lagged_input(values, lag);
    if (!out_result) return out_result.error();
    auto out = out_result.value();
    for (Count i = lag; i < values.size(); ++i) out[i - lag] = values[i] - values[i - lag];
    return out;
}

Result<linalg::Vector<Real>> simple_returns(ConstSpan<Real> prices, Count lag) noexcept {
    auto out_result = validate_lagged_input(prices, lag);
    if (!out_result) return out_result.error();
    auto out = out_result.value();
    for (Count i = lag; i < prices.size(); ++i) {
        if (prices[i] <= 0.0 || prices[i - lag] <= 0.0) {
            return Status{ErrorCode::invalid_argument, "simple_returns requires strictly positive prices"};
        }
        out[i - lag] = prices[i] / prices[i - lag] - 1.0;
    }
    return out;
}

Result<linalg::Vector<Real>> log_returns(ConstSpan<Real> prices, Count lag) noexcept {
    auto out_result = validate_lagged_input(prices, lag);
    if (!out_result) return out_result.error();
    auto out = out_result.value();
    for (Count i = lag; i < prices.size(); ++i) {
        if (prices[i] <= 0.0 || prices[i - lag] <= 0.0) {
            return Status{ErrorCode::invalid_argument, "log_returns requires strictly positive prices"};
        }
        out[i - lag] = std::log(prices[i]) - std::log(prices[i - lag]);
    }
    return out;
}

Result<linalg::Vector<Real>> rolling_mean(ConstSpan<Real> values, Count window) noexcept {
    if (window == 0 || window > values.size()) return Status{ErrorCode::invalid_argument, "rolling window must be nonzero and no larger than the input"};
    for (Real x : values) if (!std::isfinite(x)) return Status{ErrorCode::non_finite_input, "rolling_mean received a non-finite observation"};

    linalg::Vector<Real> out(values.size() - window + 1);
    Real sum = 0.0;
    for (Count i = 0; i < window; ++i) sum += values[i];
    out[0] = sum / static_cast<Real>(window);
    for (Count i = window; i < values.size(); ++i) {
        sum += values[i] - values[i - window];
        out[i - window + 1] = sum / static_cast<Real>(window);
    }
    return out;
}

Result<linalg::Vector<Real>> rolling_standard_deviation(ConstSpan<Real> values, Count window) noexcept {
    if (window == 0 || window > values.size()) return Status{ErrorCode::invalid_argument, "rolling window must be nonzero and no larger than the input"};
    for (Real x : values) if (!std::isfinite(x)) return Status{ErrorCode::non_finite_input, "rolling_standard_deviation received a non-finite observation"};

    linalg::Vector<Real> out(values.size() - window + 1);
    Real sum = 0.0;
    Real sumsq = 0.0;
    for (Count i = 0; i < window; ++i) { sum += values[i]; sumsq += values[i] * values[i]; }

    const auto variance_from_sums = [window](Real s, Real ss) noexcept {
        if (window <= 1) return 0.0;
        Real v = (ss - s * s / static_cast<Real>(window)) / static_cast<Real>(window - 1);
        return v > 0.0 ? v : 0.0;
    };
    out[0] = std::sqrt(variance_from_sums(sum, sumsq));
    for (Count i = window; i < values.size(); ++i) {
        sum += values[i] - values[i - window];
        sumsq += values[i] * values[i] - values[i - window] * values[i - window];
        out[i - window + 1] = std::sqrt(variance_from_sums(sum, sumsq));
    }
    return out;
}

Result<Real> autocorrelation(ConstSpan<Real> values, Count lag) noexcept {
    if (lag == 0 || lag >= values.size()) return Status{ErrorCode::invalid_argument, "lag must be positive and smaller than the number of observations"};
    for (Real x : values) if (!std::isfinite(x)) return Status{ErrorCode::non_finite_input, "autocorrelation received a non-finite observation"};

    Real mean = 0.0;
    for (Real x : values) mean += x;
    mean /= static_cast<Real>(values.size());

    Real numerator = 0.0;
    Real denominator = 0.0;
    for (Count i = 0; i < values.size(); ++i) {
        const Real d = values[i] - mean;
        denominator += d * d;
        if (i >= lag) numerator += d * (values[i - lag] - mean);
    }
    if (denominator == 0.0) return Status{ErrorCode::invalid_argument, "autocorrelation is undefined for a constant series"};
    return numerator / denominator;
}

} // namespace qf::timeseries
