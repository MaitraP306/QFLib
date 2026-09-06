#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::timeseries {

// First difference: x[t] - x[t-lag]. Output is compact and has n-lag elements.
[[nodiscard]] Result<linalg::Vector<Real>> difference(ConstSpan<Real> values, Count lag = 1) noexcept;

// Simple arithmetic returns: p[t] / p[t-lag] - 1. Prices must be finite and positive.
[[nodiscard]] Result<linalg::Vector<Real>> simple_returns(ConstSpan<Real> prices, Count lag = 1) noexcept;

// Log returns: log(p[t]) - log(p[t-lag]). Prices must be finite and positive.
[[nodiscard]] Result<linalg::Vector<Real>> log_returns(ConstSpan<Real> prices, Count lag = 1) noexcept;

// Rolling sample mean over a fixed window. Output has n-window+1 elements.
[[nodiscard]] Result<linalg::Vector<Real>> rolling_mean(ConstSpan<Real> values, Count window) noexcept;

// Rolling sample standard deviation over a fixed window. Output has n-window+1 elements.
[[nodiscard]] Result<linalg::Vector<Real>> rolling_standard_deviation(ConstSpan<Real> values, Count window) noexcept;

// Lag-k sample autocorrelation. Returns covariance(x[t], x[t-k]) / variance(x[t]).
[[nodiscard]] Result<Real> autocorrelation(ConstSpan<Real> values, Count lag = 1) noexcept;

} // namespace qf::timeseries
