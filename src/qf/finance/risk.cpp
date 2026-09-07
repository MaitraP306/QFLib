#include <qf/risk/risk.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace qf::risk {
namespace {

[[nodiscard]] Result<finance::Return> mean_difference(ConstSpan<finance::Return> a, ConstSpan<finance::Return> b) noexcept {
    if (a.empty() || a.size() != b.size())
        return Status{ErrorCode::invalid_argument, "return series shape mismatch"};
    Real sum = 0.0;
    for (Index i = 0; i < a.size(); ++i) {
        if (!finance::valid(a[i]) || !finance::valid(b[i]))
            return Status{ErrorCode::non_finite_input, "invalid return"};
        sum += a[i].value() - b[i].value();
    }
    return finance::Return{sum / static_cast<Real>(a.size())};
}

[[nodiscard]] Status validate_tail(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept {
    if (returns.empty() || scratch.size() < returns.size())
        return {ErrorCode::invalid_argument, "invalid returns or scratch capacity"};
    if (!finance::valid(confidence) || confidence.value() <= 0.0 || confidence.value() >= 1.0)
        return {ErrorCode::invalid_argument, "confidence must be in (0,1)"};
    for (const auto r : returns)
        if (!finance::valid(r)) return {ErrorCode::non_finite_input, "invalid return"};
    return {};
}

} // namespace

Result<Real> sharpe_ratio(finance::Return mean_return, finance::Volatility volatility, finance::InterestRate risk_free) noexcept {
    if (!finance::valid(mean_return) || !finance::valid(volatility) || !finance::valid(risk_free))
        return Status{ErrorCode::non_finite_input, "invalid Sharpe inputs"};
    if (!(volatility.value() > 0.0))
        return Status{ErrorCode::invalid_argument, "volatility must be positive"};
    return (mean_return.value() - risk_free.value()) / volatility.value();
}

Result<finance::Volatility> tracking_error(ConstSpan<finance::Return> portfolio_returns, ConstSpan<finance::Return> benchmark_returns) noexcept {
    if (portfolio_returns.empty() || portfolio_returns.size() != benchmark_returns.size())
        return Status{ErrorCode::invalid_argument, "return series shape mismatch"};
    Real mean = 0.0;
    Real m2 = 0.0;
    Count n = 0;
    for (Index i = 0; i < portfolio_returns.size(); ++i) {
        if (!finance::valid(portfolio_returns[i]) || !finance::valid(benchmark_returns[i]))
            return Status{ErrorCode::non_finite_input, "invalid return"};
        const Real x = portfolio_returns[i].value() - benchmark_returns[i].value();
        ++n;
        const Real delta = x - mean;
        mean += delta / static_cast<Real>(n);
        m2 += delta * (x - mean);
    }
    return finance::Volatility{n > 1 ? std::sqrt(std::max(0.0, m2 / static_cast<Real>(n - 1))) : 0.0};
}

Result<Real> information_ratio(ConstSpan<finance::Return> portfolio_returns, ConstSpan<finance::Return> benchmark_returns) noexcept {
    const auto active_mean = mean_difference(portfolio_returns, benchmark_returns);
    if (!active_mean) return active_mean.error();
    const auto te = tracking_error(portfolio_returns, benchmark_returns);
    if (!te) return te.error();
    if (!(te.value().value() > 0.0))
        return Status{ErrorCode::invalid_argument, "tracking error must be positive"};
    return active_mean.value().value() / te.value().value();
}

Result<finance::Return> maximum_drawdown(ConstSpan<finance::Price> prices) noexcept {
    if (prices.empty()) return Status{ErrorCode::empty_input, "empty price series"};
    Real peak = prices[0].value();
    if (!(peak > 0.0) || !std::isfinite(peak))
        return Status{ErrorCode::non_finite_input, "invalid price"};
    Real worst = 0.0;
    for (Index i = 1; i < prices.size(); ++i) {
        const Real p = prices[i].value();
        if (!(p > 0.0) || !std::isfinite(p))
            return Status{ErrorCode::non_finite_input, "invalid price"};
        peak = std::max(peak, p);
        worst = std::min(worst, p / peak - 1.0);
    }
    return finance::Return{worst};
}

finance::Return historical_var_unchecked(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept {
    std::copy(returns.begin(), returns.end(), scratch.begin());
    const Count n = returns.size();
    const Count k = static_cast<Count>(std::floor((1.0 - confidence.value()) * static_cast<Real>(n)));
    const Count index = std::min<Count>(k, n - 1);
    std::nth_element(scratch.begin(), scratch.begin() + static_cast<SignedIndex>(index), scratch.begin() + static_cast<SignedIndex>(n));
    return finance::Return{-scratch[index].value()};
}

Status historical_var_into(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch, finance::Return& output) noexcept {
    const Status status = validate_tail(returns, confidence, scratch);
    if (!status) return status;
    output = historical_var_unchecked(returns, confidence, scratch);
    return {};
}

Result<finance::Return> historical_var(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept {
    finance::Return output{};
    const Status status = historical_var_into(returns, confidence, scratch, output);
    if (!status) return status;
    return output;
}

finance::Return expected_shortfall_unchecked(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept {
    std::copy(returns.begin(), returns.end(), scratch.begin());
    const Count n = returns.size();
    const Count k = static_cast<Count>(std::floor((1.0 - confidence.value()) * static_cast<Real>(n)));
    const Count tail_count = std::max<Count>(1, k);
    std::nth_element(scratch.begin(), scratch.begin() + static_cast<SignedIndex>(tail_count), scratch.begin() + static_cast<SignedIndex>(n));
    Real sum = 0.0;
    for (Index i = 0; i < tail_count; ++i) sum += scratch[i].value();
    return finance::Return{-sum / static_cast<Real>(tail_count)};
}

Status expected_shortfall_into(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch, finance::Return& output) noexcept {
    const Status status = validate_tail(returns, confidence, scratch);
    if (!status) return status;
    output = expected_shortfall_unchecked(returns, confidence, scratch);
    return {};
}

Result<finance::Return> expected_shortfall(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept {
    finance::Return output{};
    const Status status = expected_shortfall_into(returns, confidence, scratch, output);
    if (!status) return status;
    return output;
}

} // namespace qf::risk
