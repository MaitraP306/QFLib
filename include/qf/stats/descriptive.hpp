#pragma once

#include <cmath>
#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>

namespace qf::stats {

struct DescriptiveStats {
    Real mean{};
    Real variance{};
    Real standard_deviation{};
    Real minimum{};
    Real maximum{};
    Count count{};
};

[[nodiscard]] Result<DescriptiveStats> describe(ConstSpan<Real> values) noexcept;
[[nodiscard]] Result<Real> mean(ConstSpan<Real> values) noexcept;
[[nodiscard]] Result<Real> variance(ConstSpan<Real> values) noexcept;
[[nodiscard]] Result<Real> standard_deviation(ConstSpan<Real> values) noexcept;

// Small, allocation-free state machine intended for streaming/tick updates.
struct OnlineMoments final {
    Count count{};
    Real mean{};
    Real m2{};

    [[nodiscard]] inline Status update(Real x) noexcept {
        if (!std::isfinite(x)) return Status{ErrorCode::non_finite_input, "OnlineMoments received a non-finite observation"};
        ++count;
        const Real delta = x - mean;
        mean += delta / static_cast<Real>(count);
        m2 += delta * (x - mean);
        return {};
    }

    // Caller guarantees finite x. Removes validation from a trusted hot path.
    inline void update_unchecked(Real x) noexcept {
        ++count;
        const Real delta = x - mean;
        mean += delta / static_cast<Real>(count);
        m2 += delta * (x - mean);
    }

    [[nodiscard]] Real variance() const noexcept { return count > 1 ? m2 / static_cast<Real>(count - 1) : 0.0; }
    [[nodiscard]] Real standard_deviation() const noexcept { return std::sqrt(variance()); }
};

} // namespace qf::stats
