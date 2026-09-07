#include <qf/finance/analytics.hpp>

#include <cmath>
#include <limits>

namespace qf::finance {
namespace {

[[nodiscard]] Status validate_return_inputs(ConstSpan<Return> returns) noexcept {
    if (returns.empty())
        return Status{ErrorCode::empty_input, "return series is empty"};
    for (const Return r : returns) {
        if (!valid(r))
            return Status{ErrorCode::non_finite_input, "return series contains an invalid observation"};
    }
    return {};
}

[[nodiscard]] Status validate_price_inputs(ConstSpan<Price> prices, Count lag) noexcept {
    if (lag == 0 || lag >= prices.size())
        return Status{ErrorCode::invalid_argument, "lag must be positive and smaller than the number of prices"};
    for (const Price p : prices) {
        if (!std::isfinite(p.value()) || p.value() <= 0.0)
            return Status{ErrorCode::invalid_argument, "prices must be finite and strictly positive"};
    }
    return {};
}

} // namespace

Status simple_returns_into(ConstSpan<Price> prices, Span<Return> output, Count lag) noexcept {
    auto status = validate_price_inputs(prices, lag);
    if (!status) return status;

    const Count required = prices.size() - lag;
    if (output.size() < required)
        return Status{ErrorCode::invalid_argument,  "output buffer is smaller than the return series"};

    simple_returns_into_unchecked(prices, output, lag);
    for (Count i = 0; i < required; ++i) {
        if (!std::isfinite(output[i].value()))
            return Status{ErrorCode::numerical_failure,  "simple return calculation produced a non-finite value"};
    }
    return {};
}

void simple_returns_into_unchecked(ConstSpan<Price> prices, Span<Return> output, Count lag) noexcept {
    for (Count i = lag; i < prices.size(); ++i) {
        const Real p0 = prices[i - lag].value();
        const Real p1 = prices[i].value();
        output[i - lag] = Return{p1 / p0 - 1.0};
    }
}

Status log_returns_into(ConstSpan<Price> prices, Span<LogReturn> output, Count lag) noexcept {
    auto status = validate_price_inputs(prices, lag);
    if (!status) return status;

    const Count required = prices.size() - lag;
    if (output.size() < required)
        return Status{ErrorCode::invalid_argument, "output buffer is smaller than the log-return series"};

    log_returns_into_unchecked(prices, output, lag);
    for (Count i = 0; i < required; ++i) {
        if (!std::isfinite(output[i].value()))
            return Status{ErrorCode::numerical_failure, "log return calculation produced a non-finite value"};
    }
    return {};
}

void log_returns_into_unchecked(ConstSpan<Price> prices, Span<LogReturn> output, Count lag) noexcept {
    for (Count i = lag; i < prices.size(); ++i) {
        const Real p0 = prices[i - lag].value();
        const Real p1 = prices[i].value();
        output[i - lag] = LogReturn{std::log(p1) - std::log(p0)};
    }
}

Result<linalg::Vector<Return>> simple_returns(ConstSpan<Price> prices, Count lag) noexcept {
    auto status = validate_price_inputs(prices, lag);
    if (!status) return status;

    linalg::Vector<Return> output(prices.size() - lag);
    status = simple_returns_into(prices, output.span(), lag);
    if (!status) return status;
    return output;
}

Result<linalg::Vector<LogReturn>> log_returns(ConstSpan<Price> prices, Count lag) noexcept {
    auto status = validate_price_inputs(prices, lag);
    if (!status) return status;

    linalg::Vector<LogReturn> output(prices.size() - lag);
    status = log_returns_into(prices, output.span(), lag);
    if (!status) return status;
    return output;
}

Result<Volatility> volatility(ConstSpan<Return> returns) noexcept {
    auto status = validate_return_inputs(returns);
    if (!status) return status;
    if (returns.size() < 2)
        return Status{ErrorCode::invalid_argument, "volatility requires at least two returns"};
    Real mean = 0.0;
    Real m2 = 0.0;
    Count n = 0;
    for (const Return r : returns) {
        const Real x = r.value();
        ++n;
        const Real delta = x - mean;
        mean += delta / static_cast<Real>(n);
        m2 += delta * (x - mean);
    }

    const Real variance = m2 / static_cast<Real>(n - 1);
    if (!(variance >= 0.0) || !std::isfinite(variance))
        return Status{ErrorCode::numerical_failure, "return volatility calculation failed"};
    return Volatility{std::sqrt(variance)};
}

Status zscore_into(ConstSpan<Return> returns, Span<ZScore> output) noexcept {
    auto status = validate_return_inputs(returns);
    if (!status) return status;
    if (returns.size() < 2)
        return Status{ErrorCode::invalid_argument, "z-score requires at least two returns"};
    if (output.size() < returns.size())
        return Status{ErrorCode::invalid_argument, "output buffer is smaller than the z-score series"};

    Real mean = 0.0;
    Real m2 = 0.0;
    Count n = 0;
    for (const Return r : returns) {
        const Real x = r.value();
        ++n;
        const Real delta = x - mean;
        mean += delta / static_cast<Real>(n);
        m2 += delta * (x - mean);
    }

    const Real variance = m2 / static_cast<Real>(n - 1);
    if (!(variance > 0.0) || !std::isfinite(variance))
        return Status{ErrorCode::numerical_failure, "z-score is undefined for a constant return series"};

    const Real inv_std = 1.0 / std::sqrt(variance);
    for (Count i = 0; i < returns.size(); ++i)
        output[i] = ZScore{(returns[i].value() - mean) * inv_std};
    return {};
}

Result<linalg::Vector<ZScore>> zscores(ConstSpan<Return> returns) noexcept {
    if (returns.empty())
        return Status{ErrorCode::empty_input, "return series is empty"};
    linalg::Vector<ZScore> output(returns.size());
    auto status = zscore_into(returns, output.span());
    if (!status) return status;
    return output;
}

Result<Variance> covariance(ConstSpan<Return> x, ConstSpan<Return> y) noexcept {
    if (x.size() != y.size())
        return Status{ErrorCode::invalid_argument, "return covariance requires equal-length samples"};
    if (x.size() < 2)
        return Status{ErrorCode::invalid_argument, "return covariance requires at least two observations"};

    Real mean_x = 0.0;
    Real mean_y = 0.0;
    Real c = 0.0;
    Count n = 0;

    for (Count i = 0; i < x.size(); ++i) {
        const Real xi = x[i].value();
        const Real yi = y[i].value();
        if (!std::isfinite(xi) || !std::isfinite(yi))
            return Status{ErrorCode::non_finite_input, "return covariance received an invalid observation"};

        ++n;
        const Real dx = xi - mean_x;
        const Real dy = yi - mean_y;
        mean_x += dx / static_cast<Real>(n);
        mean_y += dy / static_cast<Real>(n);
        c += dx * (yi - mean_y);
    }
    return Variance{c / static_cast<Real>(n - 1)};
}

Result<Correlation> correlation(ConstSpan<Return> x, ConstSpan<Return> y) noexcept {
    if (x.size() != y.size())
        return Status{ErrorCode::invalid_argument, "return correlation requires equal-length samples"};
    if (x.size() < 2)
        return Status{ErrorCode::invalid_argument, "return correlation requires at least two observations"};

    Real mean_x = 0.0, mean_y = 0.0;
    Real m2x = 0.0, m2y = 0.0, c = 0.0;
    Count n = 0;

    for (Count i = 0; i < x.size(); ++i) {
        const Real xi = x[i].value();
        const Real yi = y[i].value();
        if (!std::isfinite(xi) || !std::isfinite(yi))
            return Status{ErrorCode::non_finite_input, "return correlation received an invalid observation"};

        ++n;
        const Real dx = xi - mean_x;
        const Real dy = yi - mean_y;
        mean_x += dx / static_cast<Real>(n);
        mean_y += dy / static_cast<Real>(n);
        m2x += dx * (xi - mean_x);
        m2y += dy * (yi - mean_y);
        c += dx * (yi - mean_y);
    }

    const Real denominator = std::sqrt(m2x * m2y);
    if (!(denominator > 0.0) || !std::isfinite(denominator))
        return Status{ErrorCode::numerical_failure, "return correlation is undefined for zero-variance input"};

    Real rho = c / denominator;
    rho = rho > 1.0 && rho <= 1.0 + 16.0 * std::numeric_limits<Real>::epsilon() ? 1.0 : rho;
    rho = rho < -1.0 && rho >= -1.0 - 16.0 * std::numeric_limits<Real>::epsilon() ? -1.0 : rho;

    if (!std::isfinite(rho) || rho < -1.0 || rho > 1.0)
        return Status{ErrorCode::numerical_failure, "return correlation calculation failed"};
    return Correlation{rho};
}


} // namespace qf::finance
