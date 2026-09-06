#include <qf/qf.hpp>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
volatile qf::Real sink = 0.0;

template <typename Fn>
double benchmark_ns(Fn&& fn, int iterations) {
    for (int i = 0; i < 100; ++i) fn();
    const auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < iterations; ++i) fn();
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
    for (qf::Index i = 0; i < X.rows(); ++i)
        for (qf::Index j = 0; j < X.cols(); ++j)
            X(i, j) = std::sin(0.01 * static_cast<qf::Real>(i + 1) * static_cast<qf::Real>(j + 1));

    constexpr int pca_iterations = 100;
    double pca_sink = 0.0;
    const double pca_ns = benchmark_ns([&] {
        auto result = qf::stats::pca_fit(X, 4);
        if (result) pca_sink += result.value().eigenvalues[0];
    }, pca_iterations);
    sink = pca_sink;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "dot_unchecked(4096): " << dot_ns << " ns/op\n";
    std::cout << "pca_fit(256x8,k=4): " << pca_ns << " ns/op\n";
    return 0;
}
