#include <qf/math/scalar.hpp>

namespace qf::math {

double hypot2(double x, double y) noexcept { return std::hypot(x, y); }
double exp(double x) noexcept { return std::exp(x); }
double expm1(double x) noexcept { return std::expm1(x); }
double log(double x) noexcept { return std::log(x); }
double log1p(double x) noexcept { return std::log1p(x); }
double sqrt(double x) noexcept { return std::sqrt(x); }
double cbrt(double x) noexcept { return std::cbrt(x); }
double sin(double x) noexcept { return std::sin(x); }
double cos(double x) noexcept { return std::cos(x); }
double tan(double x) noexcept { return std::tan(x); }
double asin(double x) noexcept { return std::asin(x); }
double acos(double x) noexcept { return std::acos(x); }
double atan(double x) noexcept { return std::atan(x); }
double atan2(double y, double x) noexcept { return std::atan2(y, x); }
double floor(double x) noexcept { return std::floor(x); }
double ceil(double x) noexcept { return std::ceil(x); }
double round(double x) noexcept { return std::round(x); }

double log1pexp(double x) noexcept {
    if (x > 0.0) return x + std::log1p(std::exp(-x));
    return std::log1p(std::exp(x));
}

double sigmoid(double x) noexcept {
    if (x >= 0.0) return 1.0 / (1.0 + std::exp(-x));
    const double e = std::exp(x);
    return e / (1.0 + e);
}

double logit(double p) noexcept { return std::log(p / (1.0 - p)); }

} // namespace qf::math
