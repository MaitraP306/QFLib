#pragma once
#include <cstddef>
#include <cmath>
#include <qf/core/span.hpp>
#include <qf/finance/types.hpp>
namespace qf::market {
struct CurveView final { ConstSpan<finance::YearFraction> maturities{}; ConstSpan<finance::DiscountFactor> discount_factors{};
 [[nodiscard]] constexpr bool valid() const noexcept { if(maturities.empty()||maturities.size()!=discount_factors.size())return false; for(std::size_t i=0;i<maturities.size();++i){if(!finance::valid(maturities[i])||!finance::valid(discount_factors[i]))return false;if(i&&maturities[i].value()<=maturities[i-1].value())return false;}return true; }
 [[nodiscard]] constexpr std::size_t size()const noexcept{return maturities.size();}
};
[[nodiscard]] finance::DiscountFactor interpolate_discount_factor(CurveView,finance::YearFraction) noexcept;
[[nodiscard]] finance::DiscountFactor interpolate_discount_factor_unchecked(CurveView,finance::YearFraction,std::size_t&) noexcept;
}
