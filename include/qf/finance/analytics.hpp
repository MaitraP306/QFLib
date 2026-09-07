#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/finance/types.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::finance {

[[nodiscard]] Status simple_returns_into(ConstSpan<Price> prices, Span<Return> output, Count lag = 1) noexcept;

[[nodiscard]] Status log_returns_into(ConstSpan<Price> prices, Span<LogReturn> output, Count lag = 1) noexcept;

void simple_returns_into_unchecked(ConstSpan<Price> prices, Span<Return> output, Count lag = 1) noexcept;

void log_returns_into_unchecked(ConstSpan<Price> prices, Span<LogReturn> output, Count lag = 1) noexcept;

[[nodiscard]] Result<linalg::Vector<Return>> simple_returns(ConstSpan<Price> prices, Count lag = 1) noexcept;

[[nodiscard]] Result<linalg::Vector<LogReturn>> log_returns(ConstSpan<Price> prices, Count lag = 1) noexcept;

[[nodiscard]] Result<Volatility> volatility(ConstSpan<Return> returns) noexcept;

[[nodiscard]] Status zscore_into(ConstSpan<Return> returns, Span<ZScore> output) noexcept;

[[nodiscard]] Result<linalg::Vector<ZScore>> zscores(ConstSpan<Return> returns) noexcept;

[[nodiscard]] Result<Variance> covariance(ConstSpan<Return> x, ConstSpan<Return> y) noexcept;

[[nodiscard]] Result<Correlation> correlation(ConstSpan<Return> x, ConstSpan<Return> y) noexcept;


} // namespace qf::finance
