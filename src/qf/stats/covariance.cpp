#include <qf/stats/covariance.hpp>
#include <cmath>

namespace qf::stats {

Result<Real> covariance(ConstSpan<Real> x, ConstSpan<Real> y) noexcept {
    if (x.size() != y.size()) return Status{ErrorCode::invalid_argument, "covariance requires equal-length samples"};
    if (x.size() < 2) return Status{ErrorCode::invalid_argument, "covariance requires at least two observations"};
    Real mx = 0.0, my = 0.0, c = 0.0;
    Count n = 0;
    for (Index i = 0; i < x.size(); ++i) {
        const Real xi = x[i], yi = y[i];
        if (!std::isfinite(xi) || !std::isfinite(yi)) return Status{ErrorCode::non_finite_input, "covariance received a non-finite observation"};
        ++n;
        const Real dx = xi - mx;
        const Real dy = yi - my;
        mx += dx / static_cast<Real>(n);
        my += dy / static_cast<Real>(n);
        c += dx * (yi - my);
    }
    return c / static_cast<Real>(n - 1);
}

Result<Real> correlation(ConstSpan<Real> x, ConstSpan<Real> y) noexcept {
    auto c = covariance(x, y);
    if (!c) return c.error();
    if (x.size() < 2) return Status{ErrorCode::invalid_argument, "correlation requires at least two observations"};
    Real mx = 0.0, my = 0.0, m2x = 0.0, m2y = 0.0;
    Count n = 0;
    for (Index i = 0; i < x.size(); ++i) {
        const Real xi = x[i], yi = y[i];
        if (!std::isfinite(xi) || !std::isfinite(yi)) return Status{ErrorCode::non_finite_input, "correlation received a non-finite observation"};
        ++n;
        const Real dx = xi - mx, dy = yi - my;
        mx += dx / static_cast<Real>(n);
        my += dy / static_cast<Real>(n);
        m2x += dx * (xi - mx);
        m2y += dy * (yi - my);
    }
    const Real denominator = std::sqrt((m2x / static_cast<Real>(n - 1)) * (m2y / static_cast<Real>(n - 1)));
    if (!(denominator > 0.0) || !std::isfinite(denominator)) return Status{ErrorCode::numerical_failure, "correlation is undefined for zero-variance input"};
    return c.value() / denominator;
}

Result<linalg::Matrix<Real>> covariance_matrix(ConstSpan<Real> const* columns, Count column_count) {
    if (column_count == 0) return Status{ErrorCode::empty_input, "covariance_matrix requires at least one column"};
    const Count n = columns[0].size();
    if (n < 2) return Status{ErrorCode::invalid_argument, "covariance_matrix requires at least two observations"};
    for (Count j = 0; j < column_count; ++j) if (columns[j].size() != n) return Status{ErrorCode::invalid_argument, "covariance_matrix requires equal-length columns"};
    linalg::Matrix<Real> out(column_count, column_count, 0.0);
    for (Count i = 0; i < column_count; ++i) for (Count j = i; j < column_count; ++j) {
        auto c = covariance(columns[i], columns[j]);
        if (!c) return c.error();
        out(i, j) = c.value();
        out(j, i) = c.value();
    }
    return out;
}

} // namespace qf::stats
