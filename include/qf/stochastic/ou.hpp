#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/time.hpp>
#include <qf/core/types.hpp>
#include <qf/timeseries/series.hpp>

namespace qf::stochastic {

struct OUParameters final {
    Real theta{};
    Real mu{};
    Real sigma{};
    Real phi{};
    Real half_life{};
    Real dt{};
};

[[nodiscard]] Result<OUParameters> estimate_ou(ConstSpan<Real> values, Duration dt) noexcept;

[[nodiscard]] Result<OUParameters> estimate_ou(const timeseries::TimeSeries& series) noexcept;

} // namespace qf::stochastic
