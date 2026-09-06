#include <qf/stats/descriptive.hpp>

namespace qf::stats {

Result<DescriptiveStats> describe(ConstSpan<Real> values) noexcept {
    if (values.empty()) return Status{ErrorCode::empty_input, "describe requires at least one observation"};
    Real mean_value = 0.0, m2 = 0.0;
    Real minimum = values[0], maximum = values[0];
    Count n = 0;
    for (Real x : values) {
        if (!std::isfinite(x)) return Status{ErrorCode::non_finite_input, "describe received a non-finite observation"};
        ++n;
        const Real delta = x - mean_value;
        mean_value += delta / static_cast<Real>(n);
        m2 += delta * (x - mean_value);
        minimum = x < minimum ? x : minimum;
        maximum = x > maximum ? x : maximum;
    }
    const Real variance_value = n > 1 ? m2 / static_cast<Real>(n - 1) : 0.0;
    return DescriptiveStats{mean_value, variance_value, std::sqrt(variance_value), minimum, maximum, n};
}

Result<Real> mean(ConstSpan<Real> values) noexcept { auto r = describe(values); return r ? Result<Real>{r.value().mean} : Result<Real>{r.error()}; }
Result<Real> variance(ConstSpan<Real> values) noexcept { auto r = describe(values); return r ? Result<Real>{r.value().variance} : Result<Real>{r.error()}; }
Result<Real> standard_deviation(ConstSpan<Real> values) noexcept { auto r = describe(values); return r ? Result<Real>{r.value().standard_deviation} : Result<Real>{r.error()}; }

} // namespace qf::stats
