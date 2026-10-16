#pragma once
#include <array>
#include <cstdint>
#include <qf/core/span.hpp>
#include <qf/core/time.hpp>
#include <qf/finance/types.hpp>
namespace qf::market {
using InstrumentId = std::uint32_t;
using VenueId = std::uint16_t;
enum class Side : std::uint8_t {Buy,Sell};
[[nodiscard]] constexpr Int sign(Side side) noexcept {
    return side == Side::Buy ? 1 : -1;
}
inline constexpr InstrumentId invalid_instrument = 0;
inline constexpr VenueId invalid_venue = 0;
struct Quote final { Timestamp timestamp{}; InstrumentId instrument=0; VenueId venue=0; finance::Price bid{}; finance::Price ask{}; finance::Quantity bid_size{}; finance::Quantity ask_size{};
 [[nodiscard]] constexpr bool valid() const noexcept { return instrument && venue && timestamp.count() >= 0 && finance::valid(bid)&&finance::valid(ask)&&finance::valid(bid_size)&&finance::valid(ask_size)&&bid.value()>=0&&ask.value()>=bid.value()&&bid_size.value()>=0&&ask_size.value()>=0; }
 [[nodiscard]] constexpr finance::PriceDelta spread() const noexcept { return ask-bid; }
};
struct Trade final { Timestamp timestamp{}; InstrumentId instrument=0; VenueId venue=0; finance::Price price{}; finance::Quantity quantity{}; std::uint8_t aggressor_side=0;
 [[nodiscard]] constexpr bool valid() const noexcept { return instrument&&venue&&timestamp.count()>=0&&finance::valid(price)&&finance::valid(quantity)&&price.value()>=0&&quantity.value()>0&&aggressor_side<=2; }
};
struct Bar final { Timestamp timestamp{}; InstrumentId instrument=0; VenueId venue=0; finance::Price open{}; finance::Price high{}; finance::Price low{}; finance::Price close{}; finance::Quantity volume{}; std::uint32_t trade_count=0;
 [[nodiscard]] constexpr bool valid() const noexcept { return instrument&&venue&&timestamp.count()>=0&&finance::valid(open)&&finance::valid(high)&&finance::valid(low)&&finance::valid(close)&&finance::valid(volume)&&open.value()>=0&&high.value()>=low.value()&&high.value()>=open.value()&&high.value()>=close.value()&&low.value()<=open.value()&&low.value()<=close.value()&&volume.value()>=0; }
};
struct Level final { finance::Price price{}; finance::Quantity quantity{}; [[nodiscard]] constexpr bool valid() const noexcept { return finance::valid(price)&&finance::valid(quantity)&&price.value()>=0&&quantity.value()>=0; } };
template<std::size_t Depth> struct OrderBookSnapshot final { static_assert(Depth>0); Timestamp timestamp{}; InstrumentId instrument=0; VenueId venue=0; std::array<Level,Depth> bids{}; std::array<Level,Depth> asks{}; std::uint16_t bid_count=0, ask_count=0;
 [[nodiscard]] constexpr Span<const Level> bid_levels() const noexcept { return {bids.data(),bid_count}; }
 [[nodiscard]] constexpr Span<const Level> ask_levels() const noexcept { return {asks.data(),ask_count}; }
 [[nodiscard]] constexpr const Level* best_bid() const noexcept { return bid_count?&bids[0]:nullptr; }
 [[nodiscard]] constexpr const Level* best_ask() const noexcept { return ask_count?&asks[0]:nullptr; }
 [[nodiscard]] constexpr bool valid() const noexcept {
  if(!instrument||!venue||timestamp.count()<0||bid_count>Depth||ask_count>Depth)return false;
  for(std::size_t i=0;i<bid_count;++i){if(!bids[i].valid())return false;if(i&&bids[i].price.value()>=bids[i-1].price.value())return false;}
  for(std::size_t i=0;i<ask_count;++i){if(!asks[i].valid())return false;if(i&&asks[i].price.value()<=asks[i-1].price.value())return false;}
  if(bid_count&&ask_count&&bids[0].price.value()>asks[0].price.value())return false;
  return true; }
};
}
