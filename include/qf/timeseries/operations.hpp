#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::timeseries {

[[nodiscard]] Result<linalg::Vector<Real>> difference(ConstSpan<Real> values, Count lag = 1) noexcept;

[[nodiscard]] Result<linalg::Vector<Real>> simple_returns(ConstSpan<Real> prices, Count lag = 1) noexcept;

[[nodiscard]] Result<linalg::Vector<Real>> log_returns(ConstSpan<Real> prices, Count lag = 1) noexcept;

[[nodiscard]] Result<linalg::Vector<Real>> rolling_mean(ConstSpan<Real> values, Count window) noexcept;

[[nodiscard]] Result<linalg::Vector<Real>> rolling_standard_deviation(ConstSpan<Real> values, Count window) noexcept;

[[nodiscard]] Result<Real> autocorrelation(ConstSpan<Real> values, Count lag = 1) noexcept;

} // namespace qf::timeseries
