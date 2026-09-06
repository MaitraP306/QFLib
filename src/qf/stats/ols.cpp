#include <qf/stats/ols.hpp>
#include <cmath>
#include <utility>
#include <qf/linalg/algorithms.hpp>

namespace qf::stats {

Result<OLSResult> ordinary_least_squares(const linalg::Matrix<Real>& x, ConstSpan<Real> y) {
    if (x.rows() != y.size()) return Status{ErrorCode::invalid_argument, "OLS requires X.rows() == y.size()"};
    if (x.rows() == 0 || x.cols() == 0) return Status{ErrorCode::empty_input, "OLS requires non-empty design matrix"};
    if (x.rows() < x.cols()) return Status{ErrorCode::invalid_argument, "OLS requires at least as many observations as parameters"};

    auto factor = linalg::qr(x);
    if (!factor) return factor.error();
    auto qty = linalg::apply_qt(factor.value(), y);
    if (!qty) return qty.error();
    auto beta = linalg::solve_upper(factor.value().r, qty.value().span());
    if (!beta) return beta.error();

    linalg::Vector<Real> residuals(y.size());
    Real rss = 0.0, mean_y = 0.0;
    for (Real value : y) mean_y += value / static_cast<Real>(y.size());
    Real tss = 0.0;
    for (Index i = 0; i < y.size(); ++i) {
        Real fitted = 0.0;
        const Real* row = x.data() + i * x.cols();
        for (Index j = 0; j < x.cols(); ++j) fitted += row[j] * beta.value()[j];
        const Real residual = y[i] - fitted;
        residuals[i] = residual;
        rss += residual * residual;
        const Real d = y[i] - mean_y;
        tss += d * d;
    }
    return OLSResult{std::move(beta.value()), std::move(residuals), rss, tss > 0.0 ? 1.0 - rss / tss : 0.0};
}

} // namespace qf::stats
