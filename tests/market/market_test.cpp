#include <array>
#include <cassert>
#include <cmath>
#include <type_traits>
#include <qf/market.hpp>
using namespace qf; using namespace qf::finance; using namespace qf::market;
int main() {
 static_assert(std::is_trivially_copyable_v<Quote>);
 static_assert(std::is_trivially_copyable_v<Trade>);
 static_assert(std::is_trivially_copyable_v<OrderBookSnapshot<8>>);
 Quote q{Timestamp::from_nanoseconds(1),42,7,Price{99},Price{101},Quantity{30},Quantity{10}};
 assert(q.valid()); assert(mid_price_unchecked(q).value()==100); assert(std::abs(microprice_unchecked(q).value()-100.5)<1e-12); assert(std::abs(book_imbalance_unchecked(q).value()-0.5)<1e-12); assert(spread_unchecked(q).value()==2);
 std::array<Trade,2> trades{{Trade{Timestamp::from_nanoseconds(2),42,7,Price{100},Quantity{2},1},Trade{Timestamp::from_nanoseconds(3),42,7,Price{101},Quantity{1},2}}};
 assert(std::abs(vwap_unchecked(ConstSpan<Trade>{trades}).value()-301.0/3.0)<1e-12);
 OrderBookSnapshot<4> book; book.instrument=42;book.venue=7;book.timestamp=Timestamp::from_nanoseconds(4);book.bid_count=2;book.ask_count=2;book.bids[0]={Price{100},Quantity{5}};book.bids[1]={Price{99},Quantity{7}};book.asks[0]={Price{101},Quantity{4}};book.asks[1]={Price{102},Quantity{8}};assert(book.valid());assert(book.best_bid()->price.value()==100);assert(book.best_ask()->price.value()==101);
 OrderBookSnapshot<4> bad=book; bad.bids[1].price=Price{101}; assert(!bad.valid()); bad=book; bad.asks[0].price=Price{98}; assert(!bad.valid());
 std::array<YearFraction,3> t{{YearFraction{.5},YearFraction{1},YearFraction{2}}};std::array<DiscountFactor,3>d{{DiscountFactor{.99},DiscountFactor{.95},DiscountFactor{.90}}};CurveView c{ConstSpan<YearFraction>{t},ConstSpan<DiscountFactor>{d}};assert(c.valid());std::size_t h=0;auto df=interpolate_discount_factor_unchecked(c,YearFraction{1.5},h);assert(std::abs(df.value()-.925)<1e-12);assert(std::isfinite(interpolate_discount_factor(c,YearFraction{.75}).value()));
 std::array<YearFraction,2> e{{YearFraction{1},YearFraction{2}}};std::array<Price,2> k{{Price{100},Price{110}}};std::array<Volatility,4>v{{Volatility{.2},Volatility{.21},Volatility{.22},Volatility{.23}}};VolatilitySurfaceView s{ConstSpan<YearFraction>{e},ConstSpan<Price>{k},ConstSpan<Volatility>{v}};assert(s.valid());assert(s.at_unchecked(1,0).value()==.22);
 return 0;
}
