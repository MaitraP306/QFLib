#include <qf/portfolio/portfolio.hpp>

#include <cmath>

namespace qf::portfolio {

Status weighted_return_into(ConstSpan<finance::Weight> weights, ConstSpan<finance::Return> returns, finance::Return& output) noexcept {
    if (weights.empty() || returns.empty() || weights.size() != returns.size())
        return {ErrorCode::invalid_argument, "portfolio weights/returns shape mismatch"};
    for (const auto w : weights)
        if (!finance::valid(w)) return {ErrorCode::non_finite_input, "invalid portfolio weight"};
    for (const auto r : returns)
        if (!finance::valid(r)) return {ErrorCode::non_finite_input, "invalid portfolio return"};
    output = weighted_return_unchecked(weights, returns);
    return {};
}

finance::Return weighted_return_unchecked(ConstSpan<finance::Weight> weights, ConstSpan<finance::Return> returns) noexcept {
    Real sum = 0.0;
    Real compensation = 0.0;
    for (Index i = 0; i < weights.size(); ++i) {
        const Real term = weights[i].value() * returns[i].value();
        const Real y = term - compensation;
        const Real t = sum + y;
        compensation = (t - sum) - y;
        sum = t;
    }
    return finance::Return{sum};
}

Result<finance::Return> weighted_return(ConstSpan<finance::Weight> weights, ConstSpan<finance::Return> returns) noexcept {
    finance::Return output{};
    const Status status = weighted_return_into(weights, returns, output);
    if (!status) return status;
    return output;
}

Status portfolio_variance_into(ConstSpan<finance::Weight> weights, ConstSpan<Real> covariance, Count dimension, finance::Variance& output) noexcept {
    if (dimension == 0 || weights.size() != dimension || covariance.size() != dimension * dimension)
        return {ErrorCode::invalid_argument, "invalid portfolio covariance shape"};
    for (const auto w : weights)
        if (!finance::valid(w)) return {ErrorCode::non_finite_input, "invalid portfolio weight"};
    for (const Real x : covariance)
        if (!std::isfinite(x)) return {ErrorCode::non_finite_input, "invalid covariance"};
    output = portfolio_variance_unchecked(weights, covariance, dimension);
    return {};
}

finance::Variance portfolio_variance_unchecked(ConstSpan<finance::Weight> weights, ConstSpan<Real> covariance, Count dimension) noexcept {
    Real sum = 0.0;
    Real compensation = 0.0;
    for (Index i = 0; i < dimension; ++i) {
        const Real wi = weights[i].value();
        const Real diagonal = wi * covariance[i * dimension + i] * wi;
        const Real y0 = diagonal - compensation;
        const Real t0 = sum + y0;
        compensation = (t0 - sum) - y0;
        sum = t0;
        for (Index j = i + 1; j < dimension; ++j) {
            const Real term = 2.0 * wi * covariance[i * dimension + j] * weights[j].value();
            const Real y = term - compensation;
            const Real t = sum + y;
            compensation = (t - sum) - y;
            sum = t;
        }
    }
    return finance::Variance{sum};
}

Result<finance::Variance> portfolio_variance(ConstSpan<finance::Weight> weights, ConstSpan<Real> covariance, Count dimension) noexcept {
    finance::Variance output{};
    const Status status = portfolio_variance_into(weights, covariance, dimension, output);
    if (!status) return status;
    return output;
}

Status portfolio_volatility_into(ConstSpan<finance::Weight> weights, ConstSpan<Real> covariance, Count dimension, finance::Volatility& output) noexcept {
    finance::Variance variance{};
    const Status status = portfolio_variance_into(weights, covariance, dimension, variance);
    if (!status) return status;
    if (variance.value() < 0.0 || !std::isfinite(variance.value()))
        return {ErrorCode::numerical_failure, "portfolio variance is negative or non-finite"};
    output = finance::Volatility{std::sqrt(variance.value())};
    return {};
}

Result<finance::Volatility> portfolio_volatility(ConstSpan<finance::Weight> weights, ConstSpan<Real> covariance, Count dimension) noexcept {
    finance::Volatility output{};
    const Status status = portfolio_volatility_into(weights, covariance, dimension, output);
    if (!status) return status;
    return output;
}

Real factor_exposure_unchecked(ConstSpan<finance::Weight> weights, ConstSpan<Real> loadings) noexcept {
    Real sum = 0.0;
    for (Index i = 0; i < weights.size(); ++i)
        sum += weights[i].value() * loadings[i];
    return sum;
}

Result<Real> factor_exposure(ConstSpan<finance::Weight> weights, ConstSpan<Real> loadings) noexcept {
    if (weights.empty() || weights.size() != loadings.size())
        return Status{ErrorCode::invalid_argument, "factor exposure shape mismatch"};
    for (const auto w : weights)
        if (!finance::valid(w)) return Status{ErrorCode::non_finite_input, "invalid portfolio weight"};
    for (const Real x : loadings)
        if (!std::isfinite(x)) return Status{ErrorCode::non_finite_input, "invalid factor loading"};
    return factor_exposure_unchecked(weights, loadings);
}

} // namespace qf::portfolio
