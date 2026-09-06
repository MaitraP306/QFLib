#include <cassert>
#include <cmath>
#include <iostream>

#include <qf/stats.hpp>

int main() {
    {
        const qf::Real a[] = {1.0, 2.0, 3.0, 4.0, 5.0};
        auto d = qf::stats::describe(qf::ConstSpan<qf::Real>{a, 5});
        assert(d);
        assert(std::abs(d.value().mean - 3.0) < 1e-12);
        assert(std::abs(d.value().variance - 2.5) < 1e-12);
        assert(std::abs(d.value().standard_deviation - std::sqrt(2.5)) < 1e-12);

        qf::stats::OnlineMoments online;
        for (qf::Real x : a) assert(online.update(x));
        assert(online.count == 5);
        assert(std::abs(online.mean - 3.0) < 1e-12);
        assert(std::abs(online.variance() - 2.5) < 1e-12);
    }

    {
        const qf::Real x[] = {1.0, 2.0, 3.0, 4.0};
        const qf::Real y[] = {2.0, 4.0, 6.0, 8.0};
        auto c = qf::stats::covariance(
            qf::ConstSpan<qf::Real>{x, 4},
            qf::ConstSpan<qf::Real>{y, 4});
        assert(c);
        assert(std::abs(c.value() - 3.3333333333333335) < 1e-12);

        auto r = qf::stats::correlation(
            qf::ConstSpan<qf::Real>{x, 4},
            qf::ConstSpan<qf::Real>{y, 4});
        assert(r);
        assert(std::abs(r.value() - 1.0) < 1e-12);
    }

    {
        qf::linalg::Matrix<qf::Real> x(5, 2);
        const qf::Real y_data[] = {3.0, 5.0, 7.0, 9.0, 11.0};
        for (std::size_t i = 0; i < 5; ++i) {
            x(i, 0) = 1.0;
            x(i, 1) = static_cast<qf::Real>(i + 1);
        }

        auto fit = qf::stats::ordinary_least_squares(
            x, qf::ConstSpan<qf::Real>{y_data, 5});
        assert(fit);
        assert(std::abs(fit.value().coefficients[0] - 1.0) < 1e-10);
        assert(std::abs(fit.value().coefficients[1] - 2.0) < 1e-10);
        assert(fit.value().residual_sum_of_squares < 1e-18);
        assert(std::abs(fit.value().r_squared - 1.0) < 1e-12);
    }

    std::cout << "PASS: statistics and OLS tests\n";
    return 0;
}
