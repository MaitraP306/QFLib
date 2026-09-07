#include <cassert>
#include <cmath>
#include <array>
#include <iostream>

#include <qf/pricing.hpp>

using qf::finance::InterestRate;
using qf::finance::Price;
using qf::finance::Volatility;
using qf::finance::YearFraction;
using qf::finance::Yield;
using qf::pricing::OptionType;

int main() {
    using namespace qf::pricing;
    const BlackScholesParameters p{
        Price{100.0}, Price{100.0}, InterestRate{0.05}, Yield{0.0}, Volatility{0.20}, YearFraction{1.0}};

    auto call = black_scholes_price(p, OptionType::call);
    auto put = black_scholes_price(p, OptionType::put);
    assert(call && put);
    assert(std::abs(call.value().value() - 10.450583572185565) < 1e-10);
    assert(std::abs(put.value().value() - 5.573526022256971) < 1e-10);
    assert(std::abs((call.value().value() - put.value().value()) - (100.0 - 100.0 * std::exp(-0.05))) < 1e-10);

    auto g = black_scholes_greeks(p, OptionType::call);
    assert(g);
    assert(std::abs(g.value().delta - 0.6368306511756191) < 1e-10);
    assert(std::abs(g.value().gamma - 0.018762017345846895) < 1e-10);
    assert(std::abs(g.value().vega - 37.52403469169379) < 1e-9);

    const Black76Parameters b76{Price{105.12710963760242}, Price{100.0}, InterestRate{0.05}, Volatility{0.20}, YearFraction{1.0}};
    auto bprice = black76_price(b76, OptionType::call);
    assert(bprice);
    assert(bprice.value().value() > 10.0);

    auto iv = black_scholes_implied_volatility(call.value(), p, OptionType::call);
    assert(iv);
    assert(std::abs(iv.value().value() - 0.20) < 1e-8);

    std::array<qf::Real, 201> workspace{};
    Price binomial{};
    auto st = binomial_price_into_unchecked(p.spot, p.strike, p.rate, p.dividend_yield, p.volatility, p.time_to_expiry, 200, OptionType::call, false, qf::Span<qf::Real>{workspace.data(), workspace.size()}, binomial);
    assert(st);
    assert(std::abs(binomial.value() - call.value().value()) < 0.1);

    const BlackScholesParameters expiry{Price{95.0}, Price{100.0}, InterestRate{0.05}, Yield{0.0}, Volatility{0.2}, YearFraction{0.0}};
    auto exp_put = black_scholes_price(expiry, OptionType::put);
    assert(exp_put && exp_put.value().value() == 5.0);

    std::cout << "Pricing tests passed\n";
    return 0;
}
