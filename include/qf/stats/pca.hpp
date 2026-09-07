#pragma once

#include <qf/core/error.hpp>
#include <qf/core/numerical.hpp>
#include <qf/core/types.hpp>
#include <qf/linalg/matrix.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::stats {

struct PCAResult final {
    linalg::Vector<Real> mean;
    linalg::Vector<Real> eigenvalues;
    linalg::Vector<Real> explained_variance_ratio;
    linalg::Matrix<Real> components;
};

[[nodiscard]] Result<PCAResult> pca_fit(
    const linalg::Matrix<Real>& observations,
    Count n_components = 0,
    Tolerance tolerance = {});

[[nodiscard]] Result<linalg::Matrix<Real>> pca_transform(
    const PCAResult& model,
    const linalg::Matrix<Real>& observations);

[[nodiscard]] Result<linalg::Matrix<Real>> pca_fit_transform(
    const linalg::Matrix<Real>& observations,
    Count n_components = 0,
    Tolerance tolerance = {});

} // namespace qf::stats
