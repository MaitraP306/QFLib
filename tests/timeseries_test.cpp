#include <cassert>
#include <cmath>
#include <iostream>
#include <limits>

#include <qf/timeseries.hpp>

int main() {
    using namespace qf;
    using namespace qf::timeseries;

    TimeSeries series{4};
    auto t0 = Timestamp::from_nanoseconds(100);
    assert(series.append(t0, 100.0));
    assert(series.append(t0 + Duration::from_nanoseconds(10), 101.0));
    assert(series.append(t0 + Duration::from_nanoseconds(20), 103.0));
    assert(series.size() == 3);
    assert(series.values()[2] == 103.0);
    assert(!series.append(t0 + Duration::from_nanoseconds(15), 104.0));
    assert(!series.append(t0 + Duration::from_nanoseconds(30), std::numeric_limits<Real>::quiet_NaN()));

    const Real x[] = {1.0, 2.0, 4.0, 8.0};
    auto d = difference(ConstSpan<Real>{x}, 1);
    assert(d);
    assert(std::abs(d.value()[0] - 1.0) < 1e-12);
    assert(std::abs(d.value()[2] - 4.0) < 1e-12);

    auto sr = simple_returns(ConstSpan<Real>{x}, 1);
    assert(sr);
    assert(std::abs(sr.value()[0] - 1.0) < 1e-12);

    auto lr = log_returns(ConstSpan<Real>{x}, 1);
    assert(lr);
    assert(std::abs(lr.value()[0] - std::log(2.0)) < 1e-12);

    const Real y[] = {1.0, 2.0, 3.0, 4.0};
    auto rm = rolling_mean(ConstSpan<Real>{y}, 2);
    assert(rm);
    assert(std::abs(rm.value()[0] - 1.5) < 1e-12);
    assert(std::abs(rm.value()[2] - 3.5) < 1e-12);

    auto rs = rolling_standard_deviation(ConstSpan<Real>{y}, 2);
    assert(rs);
    assert(std::abs(rs.value()[0] - std::sqrt(0.5)) < 1e-12);

    const Real ar[] = {1.0, 2.0, 1.0, 2.0, 1.0, 2.0};
    auto ac = autocorrelation(ConstSpan<Real>{ar}, 1);
    assert(ac);
    assert(ac.value() < 0.0);

    const Real constant[] = {2.0, 2.0, 2.0};
    auto bad_ac = autocorrelation(ConstSpan<Real>{constant}, 1);
    assert(!bad_ac);

    std::cout << "QFLib time-series tests passed\n";
}
