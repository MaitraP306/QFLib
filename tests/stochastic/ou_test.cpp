#include <cassert>
#include <cmath>
#include <iostream>

#include <qf/stochastic.hpp>

int main() {
    using namespace qf;
    using namespace qf::stochastic;
    using qf::timeseries::TimeSeries;

    // Exact AR(1) samples from an OU process with phi = 0.8, mu = 2.0.
    // With zero innovations the regression relation is exact, which gives a
    // high-precision test of the continuous/discrete parameter conversion.
    constexpr Real phi = 0.8;
    constexpr Real mu = 2.0;
    Real values[64]{};
    values[0] = 4.0;
    for (Index i = 1; i < 64; ++i)
        values[i] = mu + phi * (values[i - 1] - mu);

    const auto dt = Duration::from_seconds(1);
    auto result = estimate_ou(ConstSpan<Real>{values}, dt);
    assert(result);

    const auto& p = result.value();
    const Real expected_theta = -std::log(phi);
    assert(std::abs(p.phi - phi) < 1e-10);
    assert(std::abs(p.theta - expected_theta) < 1e-10);
    assert(std::abs(p.mu - mu) < 1e-9);
    assert(std::abs(p.half_life - std::log(2.0) / expected_theta) < 1e-10);
    assert(p.sigma >= 0.0);

    // Non-zero innovations should produce a finite, positive diffusion
    // estimate without requiring the finite sample estimator to equal the
    // population parameter exactly.
    Real noisy[256]{};
    noisy[0] = 4.0;
    for (Index i = 1; i < 256; ++i) {
        const Real innovation = (i % 3 == 0) ? 0.02 : ((i % 3 == 1) ? -0.01 : 0.005);
        noisy[i] = mu + phi * (noisy[i - 1] - mu) + innovation;
    }
    auto noisy_result = estimate_ou(ConstSpan<Real>{noisy}, dt);
    assert(noisy_result);
    assert(noisy_result.value().sigma > 0.0);
    assert(std::isfinite(noisy_result.value().sigma));

    // TimeSeries overload and exact timestamp-spacing validation.
    TimeSeries series{9};
    const auto t0 = Timestamp::from_nanoseconds(1000);
    for (Index i = 0; i < 9; ++i)
        assert(series.append(t0 + Duration::from_seconds(static_cast<TimestampRep>(i)), values[i]));

    auto series_result = estimate_ou(series);
    assert(series_result);
    assert(std::abs(series_result.value().phi - phi) < 1e-12);

    TimeSeries irregular{9};
    assert(irregular.append(t0, values[0]));
    assert(irregular.append(t0 + Duration::from_seconds(1), values[1]));
    assert(irregular.append(t0 + Duration::from_seconds(3), values[2]));
    assert(irregular.append(t0 + Duration::from_seconds(4), values[3]));
    assert(irregular.append(t0 + Duration::from_seconds(5), values[4]));
    assert(irregular.append(t0 + Duration::from_seconds(6), values[5]));
    assert(irregular.append(t0 + Duration::from_seconds(7), values[6]));
    assert(irregular.append(t0 + Duration::from_seconds(8), values[7]));
    assert(irregular.append(t0 + Duration::from_seconds(9), values[8]));
    auto irregular_result = estimate_ou(irregular);
    assert(!irregular_result);

    // Explosive/non-mean-reverting persistence is rejected.
    const Real explosive[] = {1.0, 1.1, 1.25, 1.45, 1.75};
    auto bad = estimate_ou(ConstSpan<Real>{explosive}, dt);
    assert(!bad);

    std::cout << "QFLib OU estimation tests passed\n";
}
