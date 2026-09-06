#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

#include <qf/stats/pca.hpp>

int main() {
    // Two perfectly correlated features with different scales. The first
    // principal direction should explain essentially all sample variance.
    qf::linalg::Matrix<qf::Real> X(6, 2);
    const qf::Real x0[] = {1, 2, 3, 4, 5, 6};
    for (qf::Index i = 0; i < 6; ++i) {
        X(i, 0) = x0[i];
        X(i, 1) = 2.0 * x0[i];
    }

    auto fit = qf::stats::pca_fit(X, 2);
    assert(fit);
    assert(fit.value().eigenvalues.size() == 2);
    assert(fit.value().components.rows() == 2);
    assert(fit.value().components.cols() == 2);
    assert(std::abs(fit.value().eigenvalues[0] - 17.5) < 1e-10);
    assert(fit.value().eigenvalues[1] < 1e-10);
    assert(std::abs(fit.value().explained_variance_ratio[0] - 1.0) < 1e-12);

    const auto& c = fit.value().components;
    const qf::Real norm0 = std::sqrt(c(0, 0) * c(0, 0) + c(0, 1) * c(0, 1));
    assert(std::abs(norm0 - 1.0) < 1e-12);
    assert(std::abs(std::abs(c(0, 1) / c(0, 0)) - 2.0) < 1e-10);

    auto projected = qf::stats::pca_transform(fit.value(), X);
    assert(projected);
    assert(projected.value().rows() == 6);
    assert(projected.value().cols() == 2);

    // Centering means the projected sample mean should be approximately zero.
    qf::Real mean0 = 0.0;
    qf::Real mean1 = 0.0;
    for (qf::Index i = 0; i < 6; ++i) {
        mean0 += projected.value()(i, 0);
        mean1 += projected.value()(i, 1);
    }
    assert(std::abs(mean0 / 6.0) < 1e-12);
    assert(std::abs(mean1 / 6.0) < 1e-12);

    auto fitted = qf::stats::pca_fit_transform(X, 1);
    assert(fitted);
    assert(fitted.value().rows() == 6);
    assert(fitted.value().cols() == 1);

    auto bad_components = qf::stats::pca_fit(X, 3);
    assert(!bad_components);
    assert(bad_components.error().code() == qf::ErrorCode::invalid_argument);

    const qf::Real nan = std::numeric_limits<qf::Real>::quiet_NaN();
    X(0, 0) = nan;
    auto bad_data = qf::stats::pca_fit(X);
    assert(!bad_data);
    assert(bad_data.error().code() == qf::ErrorCode::non_finite_input);
    X(0, 0) = 1.0;

    auto one = qf::stats::pca_fit(X, 1);
    assert(one);
    assert(one.value().components.rows() == 1);
    assert(one.value().components.cols() == 2);

    std::cout << "PASS: PCA tests\n";
    return 0;
}
