#include <cassert>
#include <cmath>
#include <iostream>

#include <qf/stochastic/processes.hpp>
#include <qf/probability/random.hpp>

int main() {
    using namespace qf;
    using namespace qf::stochastic;

    constexpr Real dt = 0.25;
    constexpr Real z = 1.0;

    auto brownian = BrownianMotion::increment(dt, z);
    assert(brownian);
    assert(std::abs(brownian.value() - 0.5) < 1e-14);

    GBMParameters gbm{0.0, 0.0};
    auto gbm_result = gbm_step(100.0, gbm, 1.0, 0.0);
    assert(gbm_result);
    assert(std::abs(gbm_result.value() - 100.0) < 1e-12);

    auto ou_result = ou_step(2.0, 3.0, 2.0, 0.4, 0.5, 0.0);
    assert(ou_result);
    assert(std::abs(ou_result.value() - 2.0) < 1e-14);

    VasicekParameters vasicek{2.0, 0.03, 0.1};
    auto vasicek_result = vasicek_step(0.03, vasicek, 0.5, 0.0);
    assert(vasicek_result);
    assert(std::abs(vasicek_result.value() - 0.03) < 1e-14);

    CIRParameters cir{1.0, 0.04, 0.2};
    auto cir_result = cir_step(0.04, cir, 0.01, 0.0);
    assert(cir_result);
    assert(std::abs(cir_result.value() - 0.04) < 1e-14);

    Real path[8]{};
    GBMParameters deterministic{0.1, 0.0};
    probability::Xoshiro256StarStar rng{42};
    auto status = simulate_gbm_into_unchecked(100.0, deterministic, 1.0, rng, Span<Real>{path});
    assert(status);
    assert(std::abs(path[0] - 100.0) < 1e-14);
    assert(std::abs(path[7] - 100.0 * std::exp(0.7)) < 1e-12);

    Real ou_path[16]{};
    auto ou_status = simulate_ou_into_unchecked(4.0, 2.0, 1.0, 0.0, 0.5, rng, Span<Real>{ou_path});
    assert(ou_status);
    assert(std::abs(ou_path[15] - (1.0 + 3.0 * std::exp(-2.0 * 0.5 * 15.0))) < 1e-12);

    auto cir_negative = cir_step(0.0, cir, 1.0, -100.0);
    assert(cir_negative);
    assert(cir_negative.value() >= 0.0);

    assert(!gbm_step(0.0, gbm, 1.0, 0.0));
    assert(!ou_step(1.0, 0.0, 1.0, 0.1, 1.0, 0.0));
    assert(!cir_step(-1.0, cir, 1.0, 0.0));

    std::cout << "QFLib stochastic process tests passed\n";
}
