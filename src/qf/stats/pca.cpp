#include <qf/stats/pca.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

#include <qf/linalg/algorithms.hpp>

namespace qf::stats {

namespace {

Result<linalg::Vector<Real>> feature_means(const linalg::Matrix<Real>& X) {
    const Count n = X.rows();
    const Count p = X.cols();
    linalg::Vector<Real> means(p, 0.0);

    // Compensated summation per feature avoids avoidable loss of precision
    // when prices/returns contain a large common level.
    linalg::Vector<Real> compensation(p, 0.0);
    for (Index i = 0; i < n; ++i) {
        const Real* row = X.row(i).data();
        for (Index j = 0; j < p; ++j) {
            const Real x = row[j];
            if (!std::isfinite(x))
                return Status{ErrorCode::non_finite_input, "PCA received non-finite observations"};
            const Real y = x - compensation[j];
            const Real t = means[j] + y;
            compensation[j] = (t - means[j]) - y;
            means[j] = t;
        }
    }
    const Real inv_n = 1.0 / static_cast<Real>(n);
    for (Index j = 0; j < p; ++j) means[j] *= inv_n;
    return means;
}

} // namespace

Result<PCAResult> pca_fit(const linalg::Matrix<Real>& observations,
                          Count n_components,
                          Tolerance tolerance) {
    const Count n = observations.rows();
    const Count p = observations.cols();
    if (n == 0 || p == 0)
        return Status{ErrorCode::empty_input, "PCA requires non-empty observations"};
    if (n < 2)
        return Status{ErrorCode::invalid_argument, "PCA requires at least two observations"};
    if (n_components == 0) n_components = p;
    if (n_components > p)
        return Status{ErrorCode::invalid_argument, "n_components cannot exceed the feature count"};

    auto means_result = feature_means(observations);
    if (!means_result) return means_result.error();
    auto means = std::move(means_result.value());

    // Form the sample covariance directly from centered observations without
    // materializing an additional n x p centered matrix.
    linalg::Matrix<Real> covariance(p, p, 0.0);
    for (Index i = 0; i < n; ++i) {
        const Real* row = observations.row(i).data();
        for (Index a = 0; a < p; ++a) {
            const Real da = row[a] - means[a];
            for (Index b = a; b < p; ++b)
                covariance(a, b) += da * (row[b] - means[b]);
        }
    }
    const Real inv_dof = 1.0 / static_cast<Real>(n - 1);
    for (Index a = 0; a < p; ++a) {
        for (Index b = a; b < p; ++b) {
            covariance(a, b) *= inv_dof;
            covariance(b, a) = covariance(a, b);
        }
    }

    auto eig = linalg::symmetric_eigen(covariance, tolerance);
    if (!eig) return eig.error();

    const auto& values = eig.value().eigenvalues;
    const auto& vectors = eig.value().eigenvectors;
    PCAResult result{
        std::move(means),
        linalg::Vector<Real>(n_components),
        linalg::Vector<Real>(n_components),
        linalg::Matrix<Real>(n_components, p, 0.0)
    };

    Real total_variance = 0.0;
    for (Index i = 0; i < p; ++i)
        if (values[i] > 0.0) total_variance += values[i];

    if (!std::isfinite(total_variance))
        return Status{ErrorCode::numerical_failure, "PCA covariance spectrum is non-finite"};

    for (Index k = 0; k < n_components; ++k) {
        // Roundoff can create tiny negative eigenvalues for a PSD covariance.
        result.eigenvalues[k] = std::max(0.0, values[k]);
        result.explained_variance_ratio[k] =
            total_variance > 0.0 ? result.eigenvalues[k] / total_variance : 0.0;
        for (Index feature = 0; feature < p; ++feature)
            result.components(k, feature) = vectors(feature, k);
    }

    return result;
}

Result<linalg::Matrix<Real>> pca_transform(const PCAResult& model,
                                            const linalg::Matrix<Real>& observations) {
    const Count n = observations.rows();
    const Count p = observations.cols();
    const Count k = model.components.rows();
    if (n == 0 || p == 0 || k == 0)
        return Status{ErrorCode::empty_input, "PCA transform requires non-empty inputs"};
    if (model.mean.size() != p || model.components.cols() != p)
        return Status{ErrorCode::invalid_argument, "PCA model feature dimension does not match observations"};

    linalg::Matrix<Real> projected(n, k, 0.0);
    for (Index i = 0; i < n; ++i) {
        const Real* row = observations.row(i).data();
        Real* out = projected.row(i).data();
        for (Index feature = 0; feature < p; ++feature) {
            const Real x = row[feature];
            if (!std::isfinite(x))
                return Status{ErrorCode::non_finite_input, "PCA transform received non-finite observations"};
            const Real centered = x - model.mean[feature];
            for (Index component = 0; component < k; ++component)
                out[component] += centered * model.components(component, feature);
        }
    }
    return projected;
}

Result<linalg::Matrix<Real>> pca_fit_transform(const linalg::Matrix<Real>& observations,
                                                Count n_components,
                                                Tolerance tolerance) {
    auto model = pca_fit(observations, n_components, tolerance);
    if (!model) return model.error();
    return pca_transform(model.value(), observations);
}

} // namespace qf::stats
