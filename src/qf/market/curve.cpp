#include <qf/market/curve.hpp>
#include <cmath>
namespace qf::market {
finance::DiscountFactor interpolate_discount_factor_unchecked(CurveView c,finance::YearFraction m,std::size_t& h) noexcept{const auto n=c.size();if(n==1)return c.discount_factors[0];const double x=m.value();if(x<=c.maturities[0].value()){h=0;return c.discount_factors[0];}if(x>=c.maturities[n-1].value()){h=n-2;return c.discount_factors[n-1];}if(h>=n-1)h=n-2;while(h+1<n-1&&x>c.maturities[h+1].value())++h;while(h>0&&x<c.maturities[h].value())--h;const double t0=c.maturities[h].value(),t1=c.maturities[h+1].value();const double d0=c.discount_factors[h].value(),d1=c.discount_factors[h+1].value();return finance::DiscountFactor{d0+(x-t0)/(t1-t0)*(d1-d0)};}
finance::DiscountFactor interpolate_discount_factor(CurveView c,finance::YearFraction m) noexcept{if(!c.valid()||!finance::valid(m))return finance::DiscountFactor{NAN};std::size_t h=0;return interpolate_discount_factor_unchecked(c,m,h);}
}
