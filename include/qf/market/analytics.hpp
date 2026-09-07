#pragma once
#include <qf/core/span.hpp>
#include <qf/finance/types.hpp>
#include <qf/market/types.hpp>
namespace qf::market {
[[nodiscard]] constexpr finance::Price mid_price_unchecked(const Quote& q) noexcept{return finance::Price{0.5*(q.bid.value()+q.ask.value())};}
[[nodiscard]] constexpr finance::Price microprice_unchecked(const Quote& q) noexcept{const double d=q.bid_size.value()+q.ask_size.value();return finance::Price{d>0?(q.ask.value()*q.bid_size.value()+q.bid.value()*q.ask_size.value())/d:mid_price_unchecked(q).value()};}
[[nodiscard]] constexpr finance::Weight book_imbalance_unchecked(const Quote& q) noexcept{const double d=q.bid_size.value()+q.ask_size.value();return finance::Weight{d>0?(q.bid_size.value()-q.ask_size.value())/d:0.0};}
[[nodiscard]] constexpr finance::PriceDelta spread_unchecked(const Quote& q) noexcept{return q.spread();}
[[nodiscard]] constexpr finance::Price vwap_unchecked(ConstSpan<Trade> trades) noexcept{double n=0,q=0;for(const auto&t:trades){n+=t.price.value()*t.quantity.value();q+=t.quantity.value();}return finance::Price{q>0?n/q:0};}
}
