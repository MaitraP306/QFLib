#pragma once

#include <cmath>
#include <type_traits>

#include <qf/core/strong_scalar.hpp>

namespace qf::finance {

struct RateTag;
struct InterestRateTag;
struct YieldTag;
struct SpreadTag;
struct VolatilityTag;
struct VarianceTag;
struct ProbabilityTag;
struct DiscountFactorTag;
struct PriceTag;
struct PriceDeltaTag;
struct QuantityTag;
struct NotionalTag;
struct CashAmountTag;
struct WeightTag;
struct ZScoreTag;
struct BetaTag;
struct CorrelationTag;
struct ReturnTag;
struct LogReturnTag;
struct YearFractionTag;
struct FrequencyTag;
struct PercentTag;
struct BasisPointsTag;

using Rate          = detail::StrongScalar<RateTag>;
using InterestRate  = detail::StrongScalar<InterestRateTag>;
using Yield         = detail::StrongScalar<YieldTag>;
using Spread        = detail::StrongScalar<SpreadTag>;
using Volatility    = detail::StrongScalar<VolatilityTag>;
using Variance      = detail::StrongScalar<VarianceTag>;
using Probability   = detail::StrongScalar<ProbabilityTag>;
using DiscountFactor= detail::StrongScalar<DiscountFactorTag>;
using Price         = detail::StrongScalar<PriceTag, false>;
using PriceDelta    = detail::StrongScalar<PriceDeltaTag>;
using Quantity      = detail::StrongScalar<QuantityTag>;
using Notional      = detail::StrongScalar<NotionalTag>;
using CashAmount    = detail::StrongScalar<CashAmountTag>;
using Weight        = detail::StrongScalar<WeightTag>;
using ZScore        = detail::StrongScalar<ZScoreTag>;
using Beta          = detail::StrongScalar<BetaTag>;
using Correlation   = detail::StrongScalar<CorrelationTag>;
using Return       = detail::StrongScalar<ReturnTag, false>;
using LogReturn    = detail::StrongScalar<LogReturnTag, false>;
using YearFraction = detail::StrongScalar<YearFractionTag>;
using Frequency    = detail::StrongScalar<FrequencyTag>;
using Percent      = detail::StrongScalar<PercentTag>;
using BasisPoints   = detail::StrongScalar<BasisPointsTag>;

[[nodiscard]] constexpr InterestRate from_percent(Percent percent) noexcept {
    return InterestRate{percent.value() * 0.01};
}
[[nodiscard]] constexpr InterestRate from_basis_points(BasisPoints basis_points) noexcept {
    return InterestRate{basis_points.value() * 1e-4};
}
[[nodiscard]] constexpr Spread spread_from_percent(Percent percent) noexcept {
    return Spread{percent.value() * 0.01};
}
[[nodiscard]] constexpr Spread spread_from_basis_points(BasisPoints basis_points) noexcept {
    return Spread{basis_points.value() * 1e-4};
}
[[nodiscard]] constexpr Percent to_percent(InterestRate rate) noexcept {
    return Percent{rate.value() * 100.0};
}
[[nodiscard]] constexpr BasisPoints to_basis_points(InterestRate rate) noexcept {
    return BasisPoints{rate.value() * 10'000.0};
}
[[nodiscard]] constexpr BasisPoints to_basis_points(Spread spread) noexcept {
    return BasisPoints{spread.value() * 10'000.0};
}

} // namespace qf::finance

namespace qf::detail {

using qf::finance::CashAmount;
using qf::finance::DiscountFactor;
using qf::finance::InterestRate;
using qf::finance::Notional;
using qf::finance::Price;
using qf::finance::PriceDelta;
using qf::finance::Quantity;
using qf::finance::Rate;
using qf::finance::Variance;
using qf::finance::Volatility;
using qf::finance::Weight;
using qf::finance::YearFraction;

[[nodiscard]] constexpr PriceDelta operator-(Price lhs, Price rhs) noexcept {
    return PriceDelta{lhs.value() - rhs.value()};
}
[[nodiscard]] constexpr Price operator+(Price price, PriceDelta delta) noexcept {
    return Price{price.value() + delta.value()};
}
[[nodiscard]] constexpr Price operator+(PriceDelta delta, Price price) noexcept {
    return price + delta;
}
[[nodiscard]] constexpr Price operator-(Price price, PriceDelta delta) noexcept {
    return Price{price.value() - delta.value()};
}
[[nodiscard]] constexpr Notional operator*(Price price, Quantity quantity) noexcept {
    return Notional{price.value() * quantity.value()};
}
[[nodiscard]] constexpr Notional operator*(Quantity quantity, Price price) noexcept {
    return price * quantity;
}
[[nodiscard]] constexpr CashAmount operator*(DiscountFactor discount_factor, Notional notional) noexcept {
    return CashAmount{discount_factor.value() * notional.value()};
}
[[nodiscard]] constexpr CashAmount operator*(Notional notional, DiscountFactor discount_factor) noexcept {
    return discount_factor * notional;
}
[[nodiscard]] constexpr Real operator*(InterestRate rate, YearFraction time) noexcept {
    return rate.value() * time.value();
}
[[nodiscard]] constexpr Real operator*(YearFraction time, InterestRate rate) noexcept {
    return rate * time;
}
[[nodiscard]] constexpr Real operator*(Rate rate, YearFraction time) noexcept {
    return rate.value() * time.value();
}
[[nodiscard]] constexpr Real operator*(YearFraction time, Rate rate) noexcept {
    return rate * time;
}
[[nodiscard]] constexpr Variance operator*(Volatility volatility, Volatility volatility2) noexcept {
    return Variance{volatility.value() * volatility2.value()};
}
[[nodiscard]] constexpr Real operator*(Weight lhs, Weight rhs) noexcept {
    return lhs.value() * rhs.value();
}

} // namespace qf::detail

namespace qf::finance {

template <typename T>
requires detail::StrongScalarType<T>
[[nodiscard]] constexpr bool valid(T x) noexcept {
    return std::isfinite(x.value());
}

[[nodiscard]] constexpr bool valid(Probability x) noexcept {
    return x.value() >= 0.0 && x.value() <= 1.0;
}
[[nodiscard]] constexpr bool valid(DiscountFactor x) noexcept {
    return x.value() > 0.0 && std::isfinite(x.value());
}
[[nodiscard]] constexpr bool valid(Volatility x) noexcept {
    return x.value() >= 0.0 && std::isfinite(x.value());
}
[[nodiscard]] constexpr bool valid(Variance x) noexcept {
    return x.value() >= 0.0 && std::isfinite(x.value());
}
[[nodiscard]] constexpr bool valid(Frequency x) noexcept {
    return x.value() > 0.0 && std::isfinite(x.value());
}
[[nodiscard]] constexpr bool valid(YearFraction x) noexcept {
    return x.value() >= 0.0 && std::isfinite(x.value());
}
[[nodiscard]] constexpr bool valid(Correlation x) noexcept {
    return x.value() >= -1.0 && x.value() <= 1.0;
}
[[nodiscard]] constexpr bool valid(Return x) noexcept {
    return x.value() > -1.0 && std::isfinite(x.value());
}
[[nodiscard]] constexpr bool valid(Weight x) noexcept {
    return std::isfinite(x.value());
}

static_assert(sizeof(Rate) == sizeof(Real));
static_assert(sizeof(InterestRate) == sizeof(Real));
static_assert(sizeof(Price) == sizeof(Real));
static_assert(sizeof(Quantity) == sizeof(Real));
static_assert(sizeof(Volatility) == sizeof(Real));
static_assert(sizeof(BasisPoints) == sizeof(Real));
static_assert(std::is_trivially_copyable_v<InterestRate>);
static_assert(std::is_trivially_copyable_v<Price>);
static_assert(std::is_standard_layout_v<InterestRate>);
static_assert(std::is_standard_layout_v<Price>);

} // namespace qf::finance
