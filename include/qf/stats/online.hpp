#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <span>
#include <type_traits>

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>

namespace qf::stats {

struct OnlineMoments final {
    Count count{};
    Real mean{};
    Real m2{};

    [[nodiscard]] Status update(Real x) noexcept;
    void update_unchecked(Real x) noexcept;
    void reset() noexcept { *this = {}; }

    [[nodiscard]] Status merge(const OnlineMoments& other) noexcept;
    void merge_unchecked(const OnlineMoments& other) noexcept;

    [[nodiscard]] Real variance() const noexcept {
        return count > 1 ? m2 / static_cast<Real>(count - 1) : 0.0;
    }
    [[nodiscard]] Real standard_deviation() const noexcept {
        const Real v = variance();
        return v > 0.0 ? std::sqrt(v) : 0.0;
    }
};

struct OnlineCovariance final {
    Count count{};
    Real mean_x{};
    Real mean_y{};
    Real m2_x{};
    Real m2_y{};
    Real c{};

    [[nodiscard]] Status update(Real x, Real y) noexcept;
    void update_unchecked(Real x, Real y) noexcept;
    void reset() noexcept { *this = {}; }

    [[nodiscard]] Real covariance() const noexcept {
        return count > 1 ? c / static_cast<Real>(count - 1) : 0.0;
    }
    [[nodiscard]] Real variance_x() const noexcept {
        return count > 1 ? m2_x / static_cast<Real>(count - 1) : 0.0;
    }
    [[nodiscard]] Real variance_y() const noexcept {
        return count > 1 ? m2_y / static_cast<Real>(count - 1) : 0.0;
    }
    [[nodiscard]] Real correlation() const noexcept {
        if (count < 2 || !(m2_x > 0.0) || !(m2_y > 0.0)) return 0.0;
        return c / std::sqrt(m2_x * m2_y);
    }
};

struct OnlineRegression final {
    OnlineCovariance moments{};

    [[nodiscard]] Status update(Real x, Real y) noexcept {
        return moments.update(x, y);
    }
    void update_unchecked(Real x, Real y) noexcept {
        moments.update_unchecked(x, y);
    }
    void reset() noexcept { moments.reset(); }

    [[nodiscard]] Real slope() const noexcept {
        const Real denom = moments.m2_x;
        return denom > 0.0 ? moments.c / denom : 0.0;
    }
    [[nodiscard]] Real intercept() const noexcept {
        return moments.mean_y - slope() * moments.mean_x;
    }
    [[nodiscard]] Real r_squared() const noexcept {
        const Real rho = moments.correlation();
        return rho * rho;
    }
};

struct EWMoments final {
    Real alpha{};
    Real mean{};
    Real variance_state{};
    bool initialized{};

    explicit constexpr EWMoments(Real alpha_value) noexcept
        : alpha(alpha_value) {}

    [[nodiscard]] constexpr bool valid() const noexcept {
        return std::isfinite(alpha) && alpha > 0.0 && alpha <= 1.0;
    }

    [[nodiscard]] Status update(Real x) noexcept;
    void update_unchecked(Real x) noexcept;
    void reset() noexcept { mean = 0.0; variance_state = 0.0; initialized = false; }

    [[nodiscard]] Real variance() const noexcept { return variance_state; }
    [[nodiscard]] Real standard_deviation() const noexcept {
        return variance_state > 0.0 ? std::sqrt(variance_state) : 0.0;
    }
};

class RollingMoments final {
public:
    explicit RollingMoments(Span<Real> storage) noexcept : storage_(storage) {}

    [[nodiscard]] bool valid() const noexcept { return !storage_.empty(); }
    [[nodiscard]] Count window_size() const noexcept { return storage_.size(); }
    [[nodiscard]] Count count() const noexcept { return count_; }
    [[nodiscard]] bool full() const noexcept { return count_ == storage_.size(); }

    [[nodiscard]] Status update(Real x) noexcept;
    void update_unchecked(Real x) noexcept;
    void reset() noexcept;

    [[nodiscard]] Real mean() const noexcept { return mean_; }
    [[nodiscard]] Real variance() const noexcept {
        return count_ > 1 ? std::max(0.0, m2_) / static_cast<Real>(count_ - 1) : 0.0;
    }
    [[nodiscard]] Real standard_deviation() const noexcept {
        const Real v = variance();
        return v > 0.0 ? std::sqrt(v) : 0.0;
    }

private:
    Span<Real> storage_{};
    Count count_{};
    Count next_{};
    Real mean_{};
    Real m2_{};
};

} // namespace qf::stats
