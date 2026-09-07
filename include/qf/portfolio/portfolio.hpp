#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/finance/types.hpp>

namespace qf::portfolio {

[[nodiscard]] Status weighted_return_into(ConstSpan<finance::Weight> weights, ConstSpan<finance::Return> returns, finance::Return& output) noexcept;

[[nodiscard]] finance::Return weighted_return_unchecked(
    ConstSpan<finance::Weight> weights,
    ConstSpan<finance::Return> returns) noexcept;

[[nodiscard]] Result<finance::Return> weighted_return(
    ConstSpan<finance::Weight> weights,
    ConstSpan<finance::Return> returns) noexcept;

[[nodiscard]] Status portfolio_variance_into(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> covariance,
    Count dimension,
    finance::Variance& output) noexcept;

[[nodiscard]] finance::Variance portfolio_variance_unchecked(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> covariance,
    Count dimension) noexcept;

[[nodiscard]] Result<finance::Variance> portfolio_variance(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> covariance,
    Count dimension) noexcept;

[[nodiscard]] Status portfolio_volatility_into(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> covariance,
    Count dimension,
    finance::Volatility& output) noexcept;

[[nodiscard]] Result<finance::Volatility> portfolio_volatility(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> covariance,
    Count dimension) noexcept;

[[nodiscard]] Real factor_exposure_unchecked(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> loadings) noexcept;

[[nodiscard]] Result<Real> factor_exposure(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> loadings) noexcept;

} // namespace qf::portfolio
