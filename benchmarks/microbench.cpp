#include <qf/qf.hpp>
#include <qf/finance/streaming.hpp>
#include <qf/stats/online.hpp>
#include <qf/optimization.hpp>
#include <qf/probability.hpp>
#include <cmath>
#include <chrono>
#include <array>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
volatile qf::Real sink = 0.0;

template <typename Fn>
double benchmark_ns(Fn&& fn, int iterations) {
    for (int i = 0; i < 100; ++i) { fn(); }
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) { fn(); }
    const auto stop = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::nano>(stop - start).count() / iterations;
}
}

int main() {
    constexpr qf::Count n = 4096;
    std::vector<qf::Real> a(n), b(n);
    for (qf::Index i = 0; i < n; ++i) {
        a[i] = 0.001 * static_cast<qf::Real>(i);
        b[i] = 1.0 / (1.0 + static_cast<qf::Real>(i));
    }

    constexpr int iterations = 10000;
    qf::Real dot_total = 0.0;
    const double dot_ns = benchmark_ns([&] {
        dot_total += qf::linalg::dot_unchecked(qf::ConstSpan<qf::Real>{a}, qf::ConstSpan<qf::Real>{b});
    }, iterations);
    sink = dot_total;

    qf::linalg::Matrix<qf::Real> X(256, 8);
    for (qf::Index i = 0; i < X.rows(); ++i){
        for (qf::Index j = 0; j < X.cols(); ++j){
            X(i, j) = std::sin(0.01 * static_cast<qf::Real>(i + 1) * static_cast<qf::Real>(j + 1));
        }
    }

    constexpr int pca_iterations = 100;
    double pca_sink = 0.0;
    const double pca_ns = benchmark_ns([&] {
        auto result = qf::stats::pca_fit(X, 4);
        if (result) { pca_sink += result.value().eigenvalues[0]; }
    }, pca_iterations);
    sink = pca_sink;

    qf::stats::OnlineMoments online;
    qf::Real online_sink = 0.0;
    const double online_ns = benchmark_ns([&] {
        online.update_unchecked(0.001);
        online_sink += online.mean;
        if (online.count == 4096) { online.reset(); }
    }, iterations);
    sink = online_sink;

    std::vector<qf::Real> rolling_storage(64);
    qf::stats::RollingMoments rolling{qf::Span<qf::Real>{rolling_storage.data(), rolling_storage.size()}};
    qf::Real rolling_sink = 0.0;
    const double rolling_ns = benchmark_ns([&] {
        rolling.update_unchecked(0.001);
        rolling_sink += rolling.mean();
    }, iterations);
    sink = rolling_sink;

    qf::finance::OnlineReturnStats financial_online;
    qf::Real financial_sink = 0.0;
    const double financial_ns = benchmark_ns([&] {
        financial_online.update_unchecked(qf::finance::Return{0.0001});
        financial_sink += financial_online.volatility().value();
        if (financial_online.count() == 4096) { financial_online.reset(); }
    }, iterations);
    sink = financial_sink;

    double root_sink = 0.0;
    const double root_ns = benchmark_ns([&] {
        auto r = qf::optimization::brent([](qf::Real x) { return x * x - 2.0; }, 0.0, 2.0);
        if (r) { root_sink += r.value().root; }
    }, 10000);
    sink = root_sink;

    constexpr qf::Count opt_n = 8;
    constexpr qf::Count opt_m = 6;
    std::array<qf::Real, opt_n> opt_x{};
    std::array<qf::Real, opt_n * opt_m> opt_s{}, opt_y{};
    std::array<qf::Real, opt_m> opt_rho{}, opt_alpha{};
    std::array<qf::Real, opt_n> opt_g{}, opt_ng{}, opt_d{};
    qf::optimization::LbfgsWorkspace opt_ws{
        qf::Span<qf::Real>{opt_s.data(), opt_s.size()},
        qf::Span<qf::Real>{opt_y.data(), opt_y.size()},
        qf::Span<qf::Real>{opt_rho.data(), opt_rho.size()},
        qf::Span<qf::Real>{opt_alpha.data(), opt_alpha.size()},
        qf::Span<qf::Real>{opt_g.data(), opt_g.size()},
        qf::Span<qf::Real>{opt_ng.data(), opt_ng.size()},
        qf::Span<qf::Real>{opt_d.data(), opt_d.size()}
    };
    qf::optimization::LbfgsOptions opt_options;
    opt_options.history_size = opt_m;
    opt_options.max_iterations = 20;
    double opt_sink = 0.0;
    const double lbfgs_ns = benchmark_ns([&] {
        opt_x.fill(0.0);
        auto r = qf::optimization::lbfgs(qf::Span<qf::Real>{opt_x.data(), opt_x.size()},
            [](qf::ConstSpan<qf::Real> x, qf::Span<qf::Real> g) -> qf::Real {
                qf::Real f = 0.0;
                for (qf::Index i = 0; i < x.size(); ++i) {
                    const qf::Real d = x[i] - static_cast<qf::Real>(i + 1);
                    f += d * d;
                    g[i] = 2.0 * d;
                }
                return f;
            }, opt_ws, opt_options);
        if (r) { opt_sink += r.value().objective; }
    }, 1000);
    sink = opt_sink;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "dot_unchecked(4096): " << dot_ns << " ns/op\n";
    std::cout << "pca_fit(256x8,k=4): " << pca_ns << " ns/op\n";
    std::cout << "online_moments_update: " << online_ns << " ns/op\n";
    std::cout << "rolling_moments_update(64): " << rolling_ns << " ns/op\n";
    std::cout << "financial_return_stats_update: " << financial_ns << " ns/op\n";
    std::cout << "brent_scalar_root: " << root_ns << " ns/op\n";
    std::cout << "lbfgs(8d,m=6): " << lbfgs_ns << " ns/op\n";
    qf::probability::Xoshiro256StarStar rng{12345};
    qf::Real rng_sink = 0.0;
    const double rng_ns = benchmark_ns([&] { rng_sink += rng.uniform_open(); }, iterations);
    sink = rng_sink;

    double normal_sink = 0.0;
    const double normal_ns = benchmark_ns([&] { normal_sink += rng.normal(); }, iterations);
    sink = normal_sink;

    std::cout << "xoshiro_uniform: " << rng_ns << " ns/op\n";
    std::cout << "xoshiro_normal: " << normal_ns << " ns/op\n";

    qf::probability::Xoshiro256StarStar process_rng{987654321};
    const qf::stochastic::GBMParameters gbm_params{0.05, 0.20};
    qf::Real process_state = 100.0;
    double gbm_sink = 0.0;
    const double gbm_ns = benchmark_ns([&] {
        process_state = qf::stochastic::gbm_step_unchecked(process_state, gbm_params, 1.0 / 252.0, process_rng.normal());
        gbm_sink += process_state;
    }, iterations);
    sink = gbm_sink;

    const qf::stochastic::VasicekParameters vasicek_params{2.0, 0.03, 0.01};
    qf::Real rate_state = 0.03;
    double vasicek_sink = 0.0;
    const double vasicek_ns = benchmark_ns([&] {
        rate_state = qf::stochastic::vasicek_step_unchecked(rate_state, vasicek_params, 1.0 / 252.0, process_rng.normal());
        vasicek_sink += rate_state;
    }, iterations);
    sink = vasicek_sink;

    std::cout << "gbm_exact_step: " << gbm_ns << " ns/op\n";
    std::cout << "vasicek_exact_step: " << vasicek_ns << " ns/op\n";

    std::array<qf::finance::Weight, 8> portfolio_weights{};
    std::array<qf::Real, 64> portfolio_cov{};
    for (qf::Index i = 0; i < 8; ++i) {
        portfolio_weights[i] = qf::finance::Weight{1.0 / 8.0};
        for (qf::Index j = 0; j < 8; ++j){
            portfolio_cov[i * 8 + j] = (i == j ? 0.04 : 0.005);
        }
    }
    const double portfolio_ns = benchmark_ns([&] {
        auto v = qf::portfolio::portfolio_variance_unchecked(
            qf::ConstSpan<qf::finance::Weight>{portfolio_weights.data(), portfolio_weights.size()},
            qf::ConstSpan<qf::Real>{portfolio_cov.data(), portfolio_cov.size()}, 8
        );
        sink = v.value();
    }, iterations);

    std::array<qf::finance::Return, 64> risk_returns{};
    std::array<qf::finance::Return, 64> risk_scratch{};
    for (qf::Index i = 0; i < 64; ++i){
        risk_returns[i] = qf::finance::Return{0.0001 * static_cast<qf::Real>((i % 17) - 8)};
    }
    const double var_ns = benchmark_ns([&] {
        auto v = qf::risk::historical_var_unchecked(
            qf::ConstSpan<qf::finance::Return>{risk_returns.data(), risk_returns.size()},
            qf::finance::Probability{0.99},
            qf::Span<qf::finance::Return>{risk_scratch.data(), risk_scratch.size()}
        );
        sink = v.value();
    }, 10000);

    std::cout << "portfolio_variance(8d): " << portfolio_ns << " ns/op\n";
    std::cout << "historical_var(64): " << var_ns << " ns/op\n";
    qf::market::Quote market_quote{qf::Timestamp::from_nanoseconds(1), 42, 7,
        qf::finance::Price{99.0}, qf::finance::Price{101.0},
        qf::finance::Quantity{30.0}, qf::finance::Quantity{10.0}
    };
    double market_sink = 0.0;
    const double market_ns = benchmark_ns([&] {
        market_sink += qf::market::microprice_unchecked(market_quote).value();
        market_sink += qf::market::book_imbalance_unchecked(market_quote).value();
    }, iterations);
    sink = market_sink;
    std::cout << "market_l1_microprice_imbalance: " << market_ns << " ns/op\n";

    qf::pricing::BlackScholesParameters pricing_params{
        qf::finance::Price{100.0}, qf::finance::Price{100.0}, qf::finance::InterestRate{0.05},
        qf::finance::Yield{0.0}, qf::finance::Volatility{0.20}, qf::finance::YearFraction{1.0}
    };
    double pricing_sink = 0.0;
    const double bs_ns = benchmark_ns([&] { pricing_sink += qf::pricing::black_scholes_price_unchecked(pricing_params, qf::pricing::OptionType::call); }, iterations);
    sink = pricing_sink;

    double greeks_sink = 0.0;
    const double greeks_ns = benchmark_ns([&] {
        const auto g = qf::pricing::black_scholes_greeks_unchecked(pricing_params, qf::pricing::OptionType::call);
        greeks_sink += g.delta + g.gamma + g.vega;
    }, iterations);
    sink = greeks_sink;

    std::cout << "black_scholes_call: " << bs_ns << " ns/op\n";
    std::cout << "black_scholes_greeks: " << greeks_ns << " ns/op\n";
    return 0;
}
