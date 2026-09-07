#include <cassert>
#include <cmath>
#include <type_traits>

#include <qf/finance/streaming.hpp>

using namespace qf::finance;
using qf::Real;

namespace {
bool close(Real a, Real b, Real eps = 1e-12) { return std::abs(a - b) <= eps; }
}

int main() {
    static_assert(sizeof(OnlineReturnStats) == 3 * sizeof(Real));
    static_assert(std::is_trivially_copyable_v<OnlineReturnStats>);
    static_assert(std::is_trivially_copyable_v<OnlineReturnCovariance>);

    OnlineReturnStats stats;
    for (Return r : {Return{0.01}, Return{0.02}, Return{0.03}})
        stats.update_unchecked(r);
    assert(stats.count() == 3);
    assert(close(stats.mean().value(), 0.02));
    assert(close(stats.variance().value(), 0.0001));
    assert(close(stats.volatility().value(), 0.01));

    OnlineReturnCovariance cov;
    for (int i = 1; i <= 4; ++i)
        cov.update_unchecked(Return{0.01 * i}, Return{0.02 * i});
    assert(close(cov.correlation().value(), 1.0));
    assert(cov.covariance().value() > 0.0);
    assert(cov.volatility_x().value() > 0.0);

    OnlineReturnStats checked;
    assert(!checked.update(Return{-1.0}));
    assert(checked.count() == 0);
    assert(checked.update(Return{0.01}));
    return 0;
}
