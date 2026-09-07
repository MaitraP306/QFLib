#include <qf/optimization.hpp>
#include <qf/core/numerical.hpp>

#include <array>
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace qf;
    using namespace qf::optimization;

    {
        auto r = bisection([](Real x) { return x * x - 2.0; }, 0.0, 2.0);
        assert(r);
        assert(r.value().converged);
        assert(std::abs(r.value().root - std::sqrt(2.0)) < 1e-10);
    }
    {
        auto r = brent([](Real x) { return std::cos(x) - x; }, 0.0, 1.0);
        assert(r);
        assert(r.value().converged);
        assert(std::abs(r.value().root - 0.7390851332151607) < 1e-12);
    }
    {
        RootOptions o;
        o.absolute_tolerance = 1e-13;
        o.relative_tolerance = 0.0;
        auto r = newton([](Real x) { return x * x - 2.0; },
                        [](Real x) { return 2.0 * x; }, 1.0, o);
        assert(r);
        assert(r.value().converged);
        assert(std::abs(r.value().root - std::sqrt(2.0)) < 1e-12);
    }
    {
        std::array<Real, 2> x{0.0, 0.0};
        std::array<Real, 2 * 4> s{};
        std::array<Real, 2 * 4> y{};
        std::array<Real, 4> rho{};
        std::array<Real, 4> alpha{};
        std::array<Real, 2> g{};
        std::array<Real, 2> ng{};
        std::array<Real, 2> d{};
        LbfgsWorkspace ws{
            Span<Real>{s.data(), s.size()}, Span<Real>{y.data(), y.size()},
            Span<Real>{rho.data(), rho.size()}, Span<Real>{alpha.data(), alpha.size()},
            Span<Real>{g.data(), g.size()}, Span<Real>{ng.data(), ng.size()},
            Span<Real>{d.data(), d.size()}};
        LbfgsOptions o;
        o.history_size = 4;
        o.max_iterations = 100;
        o.gradient_tolerance = 1e-10;
        auto r = lbfgs(Span<Real>{x.data(), x.size()},
            [](ConstSpan<Real> p, Span<Real> grad) -> Real {
                const Real dx = p[0] - 3.0;
                const Real dy = p[1] + 1.0;
                grad[0] = 2.0 * dx;
                grad[1] = 4.0 * dy;
                return dx * dx + 2.0 * dy * dy;
            }, ws, o);
        assert(r);
        assert(r.value().converged);
        assert(std::abs(x[0] - 3.0) < 1e-7);
        assert(std::abs(x[1] + 1.0) < 1e-7);
    }
    {
        std::array<Real, 1> x{0.0};
        std::array<Real, 2> s{}, y{};
        std::array<Real, 2> rho{}, alpha{};
        std::array<Real, 1> g{}, ng{}, d{};
        LbfgsWorkspace ws{Span<Real>{s.data(), s.size()}, Span<Real>{y.data(), y.size()}, Span<Real>{rho.data(), rho.size()}, Span<Real>{alpha.data(), alpha.size()}, Span<Real>{g.data(), g.size()}, Span<Real>{ng.data(), ng.size()}, Span<Real>{d.data(), d.size()}};
        LbfgsOptions o; o.history_size = 3;
        auto r = lbfgs(Span<Real>{x.data(), x.size()},
            [](ConstSpan<Real> p, Span<Real> grad) -> Real { grad[0] = p[0]; return 0.5 * p[0] * p[0]; }, ws, o);
        assert(!r);
        assert(r.error().code() == ErrorCode::invalid_argument);
    }
    return 0;
}
