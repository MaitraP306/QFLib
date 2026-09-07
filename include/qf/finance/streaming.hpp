#pragma once

#include <algorithm>
#include <cmath>

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/finance/types.hpp>
#include <qf/stats/online.hpp>

namespace qf::finance {

class OnlineReturnStats final {
public:
    [[nodiscard]] Status update(Return x) noexcept {
        if (!valid(x)) return Status{ErrorCode::invalid_argument, "invalid financial return"};
        return moments_.update(x.value());
    }
    void update_unchecked(Return x) noexcept { moments_.update_unchecked(x.value()); }
    void reset() noexcept { moments_.reset(); }

    [[nodiscard]] Count count() const noexcept { return moments_.count; }
    [[nodiscard]] Real mean_value() const noexcept { return moments_.mean; }
    [[nodiscard]] Return mean() const noexcept { return Return{moments_.mean}; }
    [[nodiscard]] Variance variance() const noexcept { return Variance{moments_.variance()}; }
    [[nodiscard]] Volatility volatility() const noexcept { return Volatility{moments_.standard_deviation()}; }

private:
    stats::OnlineMoments moments_{};
};

class OnlineReturnCovariance final {
public:
    [[nodiscard]] Status update(Return x, Return y) noexcept {
        if (!valid(x) || !valid(y))
            return Status{ErrorCode::invalid_argument, "invalid financial return"};
        return moments_.update(x.value(), y.value());
    }
    void update_unchecked(Return x, Return y) noexcept {
        moments_.update_unchecked(x.value(), y.value());
    }
    void reset() noexcept { moments_.reset(); }

    [[nodiscard]] Count count() const noexcept { 
        return moments_.count; 
    }
    [[nodiscard]] Variance covariance() const noexcept { 
        return Variance{moments_.covariance()}; 
    }
    [[nodiscard]] Correlation correlation() const noexcept {
        return Correlation{moments_.correlation()};
    }
    [[nodiscard]] Volatility volatility_x() const noexcept {
        return Volatility{std::sqrt(std::max(0.0, moments_.variance_x()))};
    }
    [[nodiscard]] Volatility volatility_y() const noexcept {
        return Volatility{std::sqrt(std::max(0.0, moments_.variance_y()))};
    }

private:
    stats::OnlineCovariance moments_{};
};

} // namespace qf::finance
