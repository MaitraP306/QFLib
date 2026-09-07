#include <cassert>
#include <cmath>
#include <array>

#include <qf/portfolio/portfolio.hpp>

using namespace qf;
using namespace qf::finance;

int main() {
    const std::array<Weight, 3> w{Weight{0.5}, Weight{0.3}, Weight{0.2}};
    const std::array<Return, 3> r{Return{0.02}, Return{-0.01}, Return{0.03}};
    const auto pr = portfolio::weighted_return(ConstSpan<Weight>{w.data(), w.size()}, ConstSpan<Return>{r.data(), r.size()});
    assert(pr);
    assert(std::abs(pr.value().value() - 0.013) < 1e-15);

    const std::array<Real, 9> cov{0.04,0.01,0.0, 0.01,0.09,0.02, 0.0,0.02,0.16};
    const auto pv = portfolio::portfolio_variance(ConstSpan<Weight>{w.data(), w.size()}, ConstSpan<Real>{cov.data(), cov.size()}, 3);
    assert(pv);
    const Real expected = 0.25*0.04 + 0.09*0.09 + 0.04*0.16 + 2.0*(0.5*0.3*0.01 + 0.5*0.2*0.0 + 0.3*0.2*0.02);
    assert(std::abs(pv.value().value() - expected) < 1e-15);

    const auto vol = portfolio::portfolio_volatility(ConstSpan<Weight>{w.data(), w.size()}, ConstSpan<Real>{cov.data(), cov.size()}, 3);
    assert(vol);
    assert(std::abs(vol.value().value() - std::sqrt(expected)) < 1e-15);

    const std::array<Real, 3> loadings{1.0, -0.5, 0.25};
    const auto exposure = portfolio::factor_exposure(ConstSpan<Weight>{w.data(), w.size()}, ConstSpan<Real>{loadings.data(), loadings.size()});
    assert(exposure);
    assert(std::abs(exposure.value() - 0.4) < 1e-15);

    return 0;
}
