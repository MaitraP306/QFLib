#include <cassert>
#include <cmath>
#include <type_traits>

#include <qf/finance/analytics.hpp>

using namespace qf;
using namespace qf::finance;

namespace {

template <typename T>
bool close(T a, T b, Real eps = 1e-12) {
    return std::abs(a - b) <= eps;
}

} // namespace

int main() {
    static_assert(sizeof(Price) == sizeof(Real));
    static_assert(sizeof(Return) == sizeof(Real));
    static_assert(sizeof(LogReturn) == sizeof(Real));
    static_assert(sizeof(ZScore) == sizeof(Real));

    linalg::Vector<Price> prices{
        Price{100.0}, Price{101.0}, Price{99.99}, Price{102.0}
    };

    auto returns_result = simple_returns(prices.span());
    assert(returns_result);
    auto returns = returns_result.value();
    assert(returns.size() == 3);
    assert(close(returns[0].value(), 0.01));
    assert(close(returns[1].value(), 99.99 / 101.0 - 1.0));

    auto logs_result = log_returns(prices.span());
    assert(logs_result);
    auto logs = logs_result.value();
    assert(close(logs[0].value(), std::log(101.0) - std::log(100.0)));

    linalg::Vector<Return> output(3);
    auto status = simple_returns_into(prices.span(), output.span());
    assert(status);
    assert(close(output[2].value(), 102.0 / 99.99 - 1.0));

    linalg::Vector<Return> unchecked_output(3);
    simple_returns_into_unchecked(prices.span(), unchecked_output.span());
    assert(close(unchecked_output[1].value(), output[1].value()));

    linalg::Vector<LogReturn> unchecked_logs(3);
    log_returns_into_unchecked(prices.span(), unchecked_logs.span());
    assert(close(unchecked_logs[0].value(), logs[0].value()));

    auto vol_result = volatility(returns.span());
    assert(vol_result);
    assert(valid(vol_result.value()));
    assert(vol_result.value().value() > 0.0);

    auto z_result = zscores(returns.span());
    assert(z_result);
    assert(z_result.value().size() == returns.size());

    auto cov_result = covariance(returns.span(), returns.span());
    assert(cov_result);
    assert(cov_result.value().value() >= 0.0);

    auto corr_result = correlation(returns.span(), returns.span());
    assert(corr_result);
    assert(close(corr_result.value().value(), 1.0));

    linalg::Vector<Price> bad_prices{
        Price{100.0}, Price{0.0}, Price{101.0}
    };
    linalg::Vector<Return> bad_output(2);
    assert(!simple_returns_into(bad_prices.span(), bad_output.span()));

    linalg::Vector<Return> constant(3, Return{0.01});

    auto constant_vol = volatility(constant.span());
    assert(constant_vol);
    assert(close(constant_vol.value().value(), 0.0));

    auto constant_z = zscores(constant.span());
    assert(!constant_z);

    const Price p{100.0};
    const Quantity q{10.0};
    const Notional n = p * q;
    const DiscountFactor df{0.95};
    const CashAmount cash = df * n;
    assert(close(cash.value(), 950.0));

    return 0;
}
