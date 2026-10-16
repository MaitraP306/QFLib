#pragma once

#include <qf/core/error.hpp>
#include <qf/core/types.hpp>

#include <vector>

namespace qf::optimization {

using Vector = std::vector<qf::Real>;
using Matrix = std::vector<Vector>;

struct RegressionOptions {
    qf::Real epsilon{1.0e-8};
    qf::Real tolerance{1.0e-10};
    qf::Index maxIterations{1000};
};

struct RegressionResult {
    Vector weights;
    qf::Real objective{};
    qf::Index iterations{};
};

[[nodiscard]] qf::Result<RegressionResult> l1Positive(const Matrix& matrix, const Vector& target, const RegressionOptions& options = {});

[[nodiscard]] qf::Result<RegressionResult> l2Positive(const Matrix& matrix, const Vector& target, const RegressionOptions& options = {});

} // namespace qf::optimization