#pragma once
#include <cstddef>
#include <qf/core/span.hpp>
#include <qf/finance/types.hpp>
namespace qf::market {
struct VolatilitySurfaceView final { ConstSpan<finance::YearFraction> expiries{}; ConstSpan<finance::Price> strikes{}; ConstSpan<finance::Volatility> volatilities{};
 [[nodiscard]] constexpr bool valid()const noexcept{if(expiries.empty()||strikes.empty()||volatilities.size()!=expiries.size()*strikes.size())return false;for(std::size_t i=0;i<expiries.size();++i){if(!finance::valid(expiries[i])||(i&&expiries[i].value()<=expiries[i-1].value()))return false;}for(std::size_t i=0;i<strikes.size();++i){if(!finance::valid(strikes[i])||(i&&strikes[i].value()<=strikes[i-1].value()))return false;}for(auto v:volatilities)if(!finance::valid(v))return false;return true;}
 [[nodiscard]] constexpr std::size_t index(std::size_t e,std::size_t k)const noexcept{return e*strikes.size()+k;}
 [[nodiscard]] constexpr finance::Volatility at_unchecked(std::size_t e,std::size_t k)const noexcept{return volatilities[index(e,k)];}
};
}
