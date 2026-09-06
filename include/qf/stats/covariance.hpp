#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/linalg/matrix.hpp>

namespace qf::stats {

[[nodiscard]] Result<Real> covariance(ConstSpan<Real> x, ConstSpan<Real> y) noexcept;
[[nodiscard]] Result<Real> correlation(ConstSpan<Real> x, ConstSpan<Real> y) noexcept;
[[nodiscard]] Result<linalg::Matrix<Real>> covariance_matrix(ConstSpan<Real> const* columns, Count column_count);

} // namespace qf::stats
