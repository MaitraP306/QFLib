#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/linalg/matrix.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::stats {

struct OLSResult {
    linalg::Vector<Real> coefficients;
    linalg::Vector<Real> residuals;
    Real residual_sum_of_squares{};
    Real r_squared{};
};

[[nodiscard]] Result<OLSResult> ordinary_least_squares(const linalg::Matrix<Real>& x, ConstSpan<Real> y);

} // namespace qf::stats
