#pragma once

#include <qf/core/span.hpp>

namespace qf::math {

[[nodiscard]] double horner(double x, ConstSpan<double> coefficients) noexcept;

} // namespace qf::math
