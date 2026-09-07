#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/finance/types.hpp>

namespace qf::risk {

[[nodiscard]] Result<Real> sharpe_ratio(finance::Return mean_return, finance::Volatility volatility, finance::InterestRate risk_free = finance::InterestRate{0.0}) noexcept;

[[nodiscard]] Result<Real> information_ratio(ConstSpan<finance::Return> portfolio_returns, ConstSpan<finance::Return> benchmark_returns) noexcept;

[[nodiscard]] Result<finance::Volatility> tracking_error(ConstSpan<finance::Return> portfolio_returns, ConstSpan<finance::Return> benchmark_returns) noexcept;


[[nodiscard]] Result<finance::Return> maximum_drawdown(ConstSpan<finance::Price> prices) noexcept;

[[nodiscard]] Status historical_var_into(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch, finance::Return& output) noexcept;

[[nodiscard]] finance::Return historical_var_unchecked(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept;

[[nodiscard]] Result<finance::Return> historical_var(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept;

[[nodiscard]] Status expected_shortfall_into(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch, finance::Return& output) noexcept;

[[nodiscard]] finance::Return expected_shortfall_unchecked(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept;

[[nodiscard]] Result<finance::Return> expected_shortfall(ConstSpan<finance::Return> returns, finance::Probability confidence, Span<finance::Return> scratch) noexcept;

} // namespace qf::risk
