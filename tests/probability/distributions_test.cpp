#include <qf/probability.hpp>

#include <cassert>
#include <cmath>
#include <cstdint>

int main() {
    using namespace qf;
    using namespace qf::probability;

    {
        Normal n{};
        auto pdf0 = normal_pdf(0.0, n);
        auto cdf0 = normal_cdf(0.0, n);
        auto q50 = normal_quantile(0.5, n);
        assert(pdf0 && cdf0 && q50);
        assert(std::abs(pdf0.value() - 0.3989422804014327) < 1e-14);
        assert(std::abs(cdf0.value() - 0.5) < 1e-15);
        assert(std::abs(q50.value()) < 1e-14);
        for (Real p : {0.001, 0.025, 0.5, 0.975, 0.999}) {
            auto q = normal_quantile(p, n);
            auto c = normal_cdf(q.value(), n);
            assert(q && c);
            assert(std::abs(c.value() - p) < 2e-12);
        }
    }
    {
        Lognormal d{0.0, 0.2};
        auto q = lognormal_quantile(0.5, d);
        auto c = lognormal_cdf(1.0, d);
        assert(q && c);
        assert(std::abs(q.value() - 1.0) < 1e-14);
        assert(std::abs(c.value() - 0.5) < 1e-14);
    }
    {
        StudentT d{10.0};
        auto c = student_t_cdf(0.0, d);
        auto q = student_t_quantile(0.975, d);
        auto c2 = student_t_cdf(q.value(), d);
        assert(c && q && c2);
        assert(std::abs(c.value() - 0.5) < 1e-15);
        assert(std::abs(q.value() - 2.2281388519649385) < 2e-9);
        assert(std::abs(c2.value() - 0.975) < 2e-10);
    }
    {
        auto b = regularized_beta(0.5, 0.5, 0.5);
        assert(b);
        assert(std::abs(b.value() - 0.5) < 1e-13);
    }
    {
        auto bad = normal_quantile(0.0);
        assert(!bad && bad.error().code() == ErrorCode::invalid_argument);
        auto bad2 = student_t_pdf(0.0, StudentT{0.0});
        assert(!bad2 && bad2.error().code() == ErrorCode::invalid_argument);
    }
    {
        Xoshiro256StarStar a{42}, b{42};
        for (int i = 0; i < 100; ++i) assert(a.next_u64() == b.next_u64());
        Xoshiro256StarStar r{123};
        for (int i = 0; i < 10000; ++i) {
            const Real u = r.uniform_open();
            assert(u > 0.0 && u < 1.0);
            const Real z = r.normal();
            assert(std::isfinite(z));
        }
    }
    return 0;
}
