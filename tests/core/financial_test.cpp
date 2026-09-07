#include <cassert>
#include <cmath>
#include <type_traits>

#include <qf/finance/types.hpp>

namespace {

template <typename L, typename R>
concept Addable = requires(L lhs, R rhs) { lhs + rhs; };

template <typename L, typename R>
concept Multipliable = requires(L lhs, R rhs) { lhs * rhs; };

} // namespace

int main() {
    using namespace qf;
    using namespace qf::finance;

    static_assert(sizeof(InterestRate) == sizeof(double));
    static_assert(std::is_trivially_copyable_v<InterestRate>);
    static_assert(std::is_standard_layout_v<InterestRate>);
    static_assert(!Addable<InterestRate, Yield>);
    static_assert(!Addable<Price, Price>);
    static_assert(!Addable<Return, Return>);
    static_assert(!Multipliable<Price, InterestRate>);
    static_assert(Multipliable<Price, Quantity>);

    constexpr auto rate = from_basis_points(BasisPoints{525.0});
    static_assert(rate.value() > 0.05249 && rate.value() < 0.05251);
    static_assert(to_basis_points(rate).value() == 525.0);

    constexpr auto spread = spread_from_percent(Percent{0.25});
    static_assert(spread.value() == 0.0025);

    constexpr Price price{101.25};
    constexpr Price previous{100.75};
    constexpr auto delta = price - previous;
    static_assert(delta.value() == 0.5);
    static_assert((previous + delta).value() == price.value());

    constexpr Quantity quantity{100.0};
    constexpr auto notional = price * quantity;
    static_assert(notional.value() == 10125.0);

    constexpr DiscountFactor df{0.98};
    constexpr auto cash = df * notional;
    static_assert(cash.value() == 9922.5);

    constexpr Volatility volatility{0.20};
    constexpr auto variance = volatility * volatility;
    static_assert(variance.value() > 0.03999 && variance.value() < 0.04001);

    assert(qf::finance::valid(InterestRate{0.05}));
    assert(valid(Probability{0.25}));
    assert(!valid(Probability{1.25}));
    assert(valid(Volatility{0.0}));
    assert(!valid(Volatility{-0.01}));
    assert(valid(Correlation{-1.0}));
    assert(valid(Correlation{1.0}));
    assert(!valid(Correlation{1.01}));
    assert(valid(Frequency{252.0}));
    assert(!valid(Frequency{0.0}));
    assert(std::isfinite(to_percent(rate).value()));

    return 0;
}
