#include <cassert>
#include <cmath>
#include <array>

#include <qf/risk/risk.hpp>

using namespace qf;
using namespace qf::finance;

int main() {
    const auto sharpe = risk::sharpe_ratio(Return{0.02}, Volatility{0.1}, InterestRate{0.005});
    assert(sharpe);
    assert(std::abs(sharpe.value() - 0.15) < 1e-15);

    const std::array<Return, 4> p{Return{0.02}, Return{0.01}, Return{-0.01}, Return{0.03}};
    const std::array<Return, 4> b{Return{0.01}, Return{0.01}, Return{0.0}, Return{0.02}};
    const auto te = risk::tracking_error(ConstSpan<Return>{p.data(), p.size()}, ConstSpan<Return>{b.data(), b.size()});
    assert(te);
    const auto ir = risk::information_ratio(ConstSpan<Return>{p.data(), p.size()}, ConstSpan<Return>{b.data(), b.size()});
    assert(ir);
    assert(std::isfinite(ir.value()));

    const std::array<Price, 6> prices{Price{100}, Price{110}, Price{105}, Price{120}, Price{90}, Price{100}};
    const auto dd = risk::maximum_drawdown(ConstSpan<Price>{prices.data(), prices.size()});
    assert(dd);
    assert(std::abs(dd.value().value() + 0.25) < 1e-15);

    const std::array<Return, 10> tail{Return{0.02}, Return{0.01}, Return{-0.01}, Return{0.00}, Return{-0.02}, Return{0.03}, Return{-0.04}, Return{0.01}, Return{-0.03}, Return{0.02}};
    std::array<Return, 10> scratch{};
    const auto var = risk::historical_var(ConstSpan<Return>{tail.data(), tail.size()}, Probability{0.8}, Span<Return>{scratch.data(), scratch.size()});
    assert(var);
    assert(var.value().value() >= 0.02 - 1e-15);

    const auto es = risk::expected_shortfall(ConstSpan<Return>{tail.data(), tail.size()}, Probability{0.8}, Span<Return>{scratch.data(), scratch.size()});
    assert(es);
    assert(es.value().value() >= var.value().value() - 1e-15);

    return 0;
}
