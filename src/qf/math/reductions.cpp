#include <qf/math/reductions.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace qf::math {

double sum(ConstSpan<double> values) noexcept {
    double total = 0.0, compensation = 0.0;
    for (double x : values) {
        const double next = total + x;
        compensation += (std::abs(total) >= std::abs(x)) ? (total - next) + x : (x - next) + total;
        total = next;
    }
    return total + compensation;
}

double sum_squares(ConstSpan<double> values) noexcept {
    double total = 0.0, compensation = 0.0;
    for (double x : values) {
        const double term = x * x;
        const double next = total + term;
        compensation += (std::abs(total) >= std::abs(term)) ? (total - next) + term : (term - next) + total;
        total = next;
    }
    return total + compensation;
}

double min(ConstSpan<double> values) noexcept {
    if (values.empty()) return std::numeric_limits<double>::quiet_NaN();
    return *std::min_element(values.begin(), values.end());
}

double max(ConstSpan<double> values) noexcept {
    if (values.empty()) return std::numeric_limits<double>::quiet_NaN();
    return *std::max_element(values.begin(), values.end());
}

double mean(ConstSpan<double> values) noexcept {
    if (values.empty()) return std::numeric_limits<double>::quiet_NaN();
    return sum(values) / static_cast<double>(values.size());
}

double rms(ConstSpan<double> values) noexcept {
    if (values.empty()) return std::numeric_limits<double>::quiet_NaN();
    return std::sqrt(sum_squares(values) / static_cast<double>(values.size()));
}

} // namespace qf::math
