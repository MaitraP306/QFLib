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
struct PnLTag;
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

// Migrating Price, Quantity, Notional
struct ScaledPriceTag;
struct ScaledPriceDeltaTag;
struct ScaledQuantityTag;
struct ScaledNotionalTag;
struct ScaleTag;
struct ScaledPnLTag;

using ScaledPrice = detail::StrongScalar<ScaledPriceTag , false, Int>;
using ScaledPriceDelta = detail::StrongScalar<ScaledPriceDeltaTag , true, Int>;
using ScaledQuantity = detail::StrongScalar<ScaledQuantityTag, true, Int>;
using ScaledNotional = detail::StrongScalar<ScaledNotionalTag , true, Int>;
using Scale = detail::StrongScalar<ScaleTag, false, Int>;
using ScaledPnL = detail::StrongScalar<ScaledPnLTag, true, Int>;

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
using Return        = detail::StrongScalar<ReturnTag, false>;
using LogReturn     = detail::StrongScalar<LogReturnTag, false>;
using YearFraction  = detail::StrongScalar<YearFractionTag>;
using Frequency     = detail::StrongScalar<FrequencyTag>;
using Percent       = detail::StrongScalar<PercentTag>;
using BasisPoints   = detail::StrongScalar<BasisPointsTag>;
using PnL           = detail::StrongScalar<PnLTag, true>;

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
[[nodiscard]] constexpr CashAmount cash_from_notional(Notional notional) noexcept {
        return CashAmount{notional.value()};
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
using qf::finance::PnL;
using qf::finance::Return;


// Migrating Price, Quantity
using qf::finance::ScaledPriceDelta;
using qf::finance::ScaledPrice;
using qf::finance::ScaledQuantity;
using qf::finance::ScaledNotional;
using qf::finance::Scale;
using qf::finance::ScaledPnL;
[[nodiscard]] constexpr ScaledPriceDelta operator-(ScaledPrice lhs, ScaledPrice rhs) noexcept {
    return ScaledPriceDelta{lhs.value() - rhs.value()};
}
[[nodiscard]] constexpr ScaledPrice operator+(ScaledPrice lhs, ScaledPriceDelta rhs) noexcept {
    return ScaledPrice{lhs.value() + rhs.value()};
}
[[nodiscard]] constexpr ScaledPrice operator+(ScaledPriceDelta lhs, ScaledPrice rhs) noexcept {
    return ScaledPrice{lhs.value() + rhs.value()};
}
[[nodiscard]] constexpr ScaledPrice operator-(ScaledPrice lhs, ScaledPriceDelta rhs) noexcept {
    return ScaledPrice{lhs.value() - rhs.value()};
}
[[nodiscard]] constexpr ScaledNotional operator*(ScaledPrice lhs, ScaledQuantity rhs) noexcept {
    return ScaledNotional{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr ScaledNotional operator*(ScaledQuantity lhs, ScaledPrice rhs) noexcept {
    return ScaledNotional{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr ScaledPnL operator*(ScaledPriceDelta lhs, ScaledQuantity rhs) noexcept {
    return ScaledPnL{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr ScaledPnL operator*(ScaledQuantity lhs, ScaledPriceDelta rhs) noexcept {
    return ScaledPnL{lhs.value() * rhs.value()};
}
[[nodiscard]] constexpr Notional operator/(ScaledNotional lhs, Scale rhs) noexcept {
    return Notional{static_cast<Real>(lhs.value())/static_cast<Real>(rhs.value())};
}
[[nodiscard]] constexpr PnL operator/(ScaledPnL lhs, Scale rhs) noexcept {
    return PnL{static_cast<Real>(lhs.value())/static_cast<Real>(rhs.value())};
}
[[nodiscard]] constexpr Quantity operator/(ScaledQuantity lhs, Scale rhs) noexcept {
    return Quantity{static_cast<Real>(lhs.value())/static_cast<Real>(rhs.value())};
}
[[nodiscard]] constexpr Price operator/(ScaledPrice lhs, Scale rhs) noexcept {
    return Price{static_cast<Real>(lhs.value())/static_cast<Real>(rhs.value())};
}
[[nodiscard]] constexpr PriceDelta operator/(ScaledPriceDelta lhs, Scale rhs) noexcept {
    return PriceDelta{static_cast<Real>(lhs.value())/static_cast<Real>(rhs.value())};
}
[[nodiscard]] constexpr ScaledPrice operator*(Price lhs, Scale rhs) noexcept {
    return ScaledPrice{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledQuantity operator*(Quantity lhs, Scale rhs) noexcept {
    return ScaledQuantity{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledQuantity operator*(Scale lhs, Quantity rhs) noexcept {
    return ScaledQuantity{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledPrice operator*(Scale lhs, Price rhs) noexcept {
    return ScaledPrice{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledPriceDelta operator*(PriceDelta lhs, Scale rhs) noexcept {
    return ScaledPriceDelta{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledPriceDelta operator*(Scale lhs, PriceDelta rhs) noexcept {
    return ScaledPriceDelta{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledNotional operator*(Notional lhs, Scale rhs) noexcept {
    return ScaledNotional{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledNotional operator*(Scale lhs, Notional rhs) noexcept {
    return ScaledNotional{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledPnL operator*(PnL lhs, Scale rhs) noexcept {
    return ScaledPnL{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr ScaledPnL operator*(Scale lhs, PnL rhs) noexcept {
    return ScaledPnL{static_cast<Int>(lhs.value()*rhs.value())};
}
[[nodiscard]] constexpr Notional operator*(Price lhs, ScaledQuantity rhs) noexcept {
    return Notional{lhs.value()*rhs.value()};
}
[[nodiscard]] constexpr Notional operator*(ScaledQuantity lhs, Price rhs) noexcept {
    return Notional{lhs.value()*rhs.value()};
}
[[nodiscard]] constexpr Price operator/(Notional lhs, ScaledQuantity rhs) noexcept {
    return Price{lhs.value()/rhs.value()};
}
[[nodiscard]] constexpr Price operator/(Notional lhs, Quantity rhs) noexcept {
    return Price{lhs.value()/rhs.value()};
}
// [[nodiscard]] constexpr ScaledQuantity operator/(Notional lhs, Price rhs) noexcept {
//     return ScaledQuantity{static_cast<Int>(lhs.value()/rhs.value())};
// }
[[nodiscard]] constexpr Quantity operator/(Notional lhs, Price rhs) noexcept {
    return Quantity{lhs.value()/rhs.value()};
}
[[nodiscard]] constexpr PnL operator*(PriceDelta lhs, ScaledQuantity rhs) noexcept {
    return PnL{lhs.value()*rhs.value()};
}
[[nodiscard]] constexpr PnL operator*(ScaledQuantity lhs, PriceDelta rhs) noexcept {
    return PnL{lhs.value()*rhs.value()};
}
[[nodiscard]] constexpr ScaledQuantity operator*(ScaledQuantity lhs, Weight rhs) noexcept {
    return ScaledQuantity{lhs.value()*rhs.value()};
}
[[nodiscard]] constexpr ScaledQuantity operator*(Weight lhs, ScaledQuantity rhs) noexcept {
    return ScaledQuantity{lhs.value()*rhs.value()};
}
[[nodiscard]] constexpr Return get_return_from_cashamount(CashAmount lhs, CashAmount rhs) noexcept {
    return Return{lhs.value()/rhs.value() - 1.0};
}
[[nodiscard]] constexpr Return get_return_from_price(ScaledPrice lhs, ScaledPrice rhs) noexcept {
    return Return{static_cast<Real>(lhs.value())/rhs.value() - 1.0};
}
[[nodiscard]] constexpr Return get_return_from_price(Price lhs, Price rhs) noexcept {
    return Return{lhs.value()/rhs.value() - 1.0};
}
[[nodiscard]] constexpr Return pow(Return lhs, Real pow) noexcept {
    return Return{std::pow(1.0+lhs.value() , pow) - 1.0};
}
[[nodiscard]] constexpr Return operator-(Return lhs) noexcept {
    return Return{-lhs.value()};
}
[[nodiscard]] constexpr Quantity operator*(Quantity lhs , Weight rhs) noexcept {
    return Quantity{lhs.value()*rhs.value()};
}






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
[[nodiscard]] constexpr PnL operator*(PriceDelta price_delta, Quantity quantity) noexcept {
    return PnL{price_delta.value() * quantity.value()};
}
[[nodiscard]] constexpr PnL operator*(Quantity quantity, PriceDelta price_delta) noexcept {
    return price_delta * quantity;
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



[[nodiscard]] constexpr bool valid(ScaledPrice price) noexcept {
    return price.value()>0;
}
[[nodiscard]] constexpr bool valid(Scale s) noexcept {
    return s.value() > 0;
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
static_assert(sizeof(PnL) == sizeof(Real));
static_assert(std::is_trivially_copyable_v<PnL>);
static_assert(std::is_standard_layout_v<PnL>);



static_assert(std::is_same_v<ScaledPrice::value_type,Int>);
static_assert(std::is_same_v<ScaledPriceDelta::value_type,Int>);
static_assert(std::is_same_v<ScaledQuantity::value_type,Int>);
static_assert(std::is_same_v<ScaledNotional::value_type,Int>);
static_assert(std::is_same_v<Scale::value_type,Int>);
static_assert(std::is_same_v<ScaledPnL::value_type,Int>);
static_assert(sizeof(ScaledPrice) == sizeof(Int));
static_assert(sizeof(ScaledPriceDelta) == sizeof(Int));
static_assert(sizeof(ScaledQuantity) == sizeof(Int));
static_assert(sizeof(ScaledNotional) == sizeof(Int));
static_assert(sizeof(Scale) == sizeof(Int));
static_assert(sizeof(ScaledPnL) == sizeof(Int));
static_assert(std::is_trivially_copyable_v<ScaledPrice>);
static_assert(std::is_trivially_copyable_v<ScaledPriceDelta>);
static_assert(std::is_trivially_copyable_v<ScaledQuantity>);
static_assert(std::is_trivially_copyable_v<ScaledNotional>);
static_assert(std::is_trivially_copyable_v<Scale>);
static_assert(std::is_trivially_copyable_v<ScaledPnL>);
static_assert(std::is_standard_layout_v<ScaledPrice>);
static_assert(std::is_standard_layout_v<ScaledPriceDelta>);
static_assert(std::is_standard_layout_v<ScaledQuantity>);
static_assert(std::is_standard_layout_v<ScaledNotional>);
static_assert(std::is_standard_layout_v<Scale>);
static_assert(std::is_standard_layout_v<ScaledPnL>);
} // namespace qf::finance