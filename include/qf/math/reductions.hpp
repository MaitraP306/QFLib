#pragma once

#include <qf/core/span.hpp>

namespace qf::math {

[[nodiscard]] double sum(ConstSpan<double> values) noexcept;
[[nodiscard]] double sum_squares(ConstSpan<double> values) noexcept;
[[nodiscard]] double min(ConstSpan<double> values) noexcept;
[[nodiscard]] double max(ConstSpan<double> values) noexcept;
[[nodiscard]] double mean(ConstSpan<double> values) noexcept;
[[nodiscard]] double rms(ConstSpan<double> values) noexcept;

} // namespace qf::math
