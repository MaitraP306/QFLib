#include <qf/stats/online.hpp>

#include <algorithm>
#include <cmath>

namespace qf::stats {

Status OnlineMoments::update(Real x) noexcept {
    if (!std::isfinite(x))
        return Status{ErrorCode::non_finite_input, "OnlineMoments received a non-finite observation"};
    update_unchecked(x);
    return {};
}

void OnlineMoments::update_unchecked(Real x) noexcept {
    ++count;
    const Real delta = x - mean;
    mean += delta / static_cast<Real>(count);
    m2 += delta * (x - mean);
}

Status OnlineMoments::merge(const OnlineMoments& other) noexcept {
    if (other.count == 0) return {};
    if (!std::isfinite(other.mean) || !std::isfinite(other.m2))
        return Status{ErrorCode::non_finite_input, "OnlineMoments cannot merge invalid state"};
    merge_unchecked(other);
    return {};
}

void OnlineMoments::merge_unchecked(const OnlineMoments& other) noexcept {
    if (other.count == 0) return;
    if (count == 0) { *this = other; return; }
    const Real n1 = static_cast<Real>(count);
    const Real n2 = static_cast<Real>(other.count);
    const Real n = n1 + n2;
    const Real delta = other.mean - mean;
    m2 += other.m2 + delta * delta * (n1 * n2 / n);
    mean += delta * (n2 / n);
    count += other.count;
}

Status OnlineCovariance::update(Real x, Real y) noexcept {
    if (!std::isfinite(x) || !std::isfinite(y))
        return Status{ErrorCode::non_finite_input, "OnlineCovariance received a non-finite observation"};
    update_unchecked(x, y);
    return {};
}

void OnlineCovariance::update_unchecked(Real x, Real y) noexcept {
    ++count;
    const Real dx = x - mean_x;
    const Real dy = y - mean_y;
    mean_x += dx / static_cast<Real>(count);
    mean_y += dy / static_cast<Real>(count);
    m2_x += dx * (x - mean_x);
    m2_y += dy * (y - mean_y);
    c += dx * (y - mean_y);
}

Status EWMoments::update(Real x) noexcept {
    if (!valid())
        return Status{ErrorCode::invalid_argument, "EWMA alpha must be finite and in (0, 1]"};
    if (!std::isfinite(x))
        return Status{ErrorCode::non_finite_input, "EWMoments received a non-finite observation"};
    update_unchecked(x);
    return {};
}

void EWMoments::update_unchecked(Real x) noexcept {
    if (!initialized) {
        mean = x;
        variance_state = 0.0;
        initialized = true;
        return;
    }
    const Real delta = x - mean;
    mean += alpha * delta;
    variance_state = (1.0 - alpha) * (variance_state + alpha * delta * delta);
    if (variance_state < 0.0) variance_state = 0.0;
}

Status RollingMoments::update(Real x) noexcept {
    if (!valid())
        return Status{ErrorCode::invalid_argument, "RollingMoments requires non-empty storage"};
    if (!std::isfinite(x))
        return Status{ErrorCode::non_finite_input, "RollingMoments received a non-finite observation"};
    update_unchecked(x);
    return {};
}

void RollingMoments::update_unchecked(Real x) noexcept {
    if (count_ < storage_.size()) {
        storage_[count_] = x;
        ++count_;
        const Real delta = x - mean_;
        mean_ += delta / static_cast<Real>(count_);
        m2_ += delta * (x - mean_);
        if (count_ == storage_.size()) next_ = 0;
        return;
    }

    const Real old = storage_[next_];
    const Real n = static_cast<Real>(count_);
    const Real new_n = n - 1.0;
    const Real new_mean = (mean_ * n - old) / new_n;
    m2_ -= (old - mean_) * (old - new_mean);
    mean_ = new_mean;

    storage_[next_] = x;
    next_ = (next_ + 1 == storage_.size()) ? 0 : next_ + 1;

    const Real delta = x - mean_;
    mean_ += delta / n;
    m2_ += delta * (x - mean_);
    if (m2_ < 0.0 && m2_ > -64.0 * std::numeric_limits<Real>::epsilon()) m2_ = 0.0;
}

void RollingMoments::reset() noexcept {
    count_ = 0;
    next_ = 0;
    mean_ = 0.0;
    m2_ = 0.0;
}

} // namespace qf::stats
