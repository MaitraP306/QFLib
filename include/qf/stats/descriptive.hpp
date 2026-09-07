#pragma once

#include <cmath>
#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/stats/online.hpp>

namespace qf::stats {

struct DescriptiveStats {
    Real mean{};
    Real variance{};
    Real standard_deviation{};
    Real minimum{};
    Real maximum{};
    Count count{};
};

[[nodiscard]] Result<DescriptiveStats> describe(ConstSpan<Real> values) noexcept;
[[nodiscard]] Result<Real> mean(ConstSpan<Real> values) noexcept;
[[nodiscard]] Result<Real> variance(ConstSpan<Real> values) noexcept;
[[nodiscard]] Result<Real> standard_deviation(ConstSpan<Real> values) noexcept;

} // namespace qf::stats
