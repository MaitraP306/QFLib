#include <cassert>
#include <cmath>
#include <vector>

#include <qf/stats/online.hpp>

using namespace qf;
using namespace qf::stats;

namespace {
bool close(Real a, Real b, Real eps = 1e-12) { return std::abs(a - b) <= eps; }
}

int main() {
    OnlineMoments m;
    for (Real x : {1.0, 2.0, 3.0, 4.0, 5.0}) m.update_unchecked(x);
    assert(m.count == 5);
    assert(close(m.mean, 3.0));
    assert(close(m.variance(), 2.5));
    assert(close(m.standard_deviation(), std::sqrt(2.5)));

    OnlineMoments a, b;
    for (Real x : {1.0, 2.0, 3.0}) a.update_unchecked(x);
    for (Real x : {4.0, 5.0}) b.update_unchecked(x);
    assert(a.merge(b));
    assert(a.count == 5);
    assert(close(a.mean, 3.0));
    assert(close(a.variance(), 2.5));

    OnlineCovariance cov;
    for (Real x : {1.0, 2.0, 3.0, 4.0}) cov.update_unchecked(x, 2.0 * x + 1.0);
    assert(close(cov.covariance(), 3.3333333333333335));
    assert(close(cov.correlation(), 1.0));

    OnlineRegression reg;
    for (Real x : {1.0, 2.0, 3.0, 4.0}) reg.update_unchecked(x, 2.0 * x + 1.0);
    assert(close(reg.slope(), 2.0));
    assert(close(reg.intercept(), 1.0));
    assert(close(reg.r_squared(), 1.0));

    EWMoments ewma{0.5};
    assert(ewma.update(1.0));
    assert(ewma.update(3.0));
    assert(close(ewma.mean, 2.0));
    assert(close(ewma.variance(), 1.0));

    EWMoments bad{0.0};
    assert(!bad.update(1.0));

    std::vector<Real> storage(3);
    RollingMoments rolling{Span<Real>{storage.data(), storage.size()}};
    rolling.update_unchecked(1.0);
    rolling.update_unchecked(2.0);
    rolling.update_unchecked(3.0);
    assert(rolling.full());
    assert(close(rolling.mean(), 2.0));
    assert(close(rolling.variance(), 1.0));
    rolling.update_unchecked(4.0); // window = {2,3,4}
    assert(close(rolling.mean(), 3.0));
    assert(close(rolling.variance(), 1.0));
    rolling.reset();
    assert(rolling.count() == 0);

    assert(!m.update(std::numeric_limits<Real>::quiet_NaN()));
    return 0;
}
