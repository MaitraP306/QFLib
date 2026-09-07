#pragma once

#include <cassert>
#include <vector>

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/time.hpp>
#include <qf/core/types.hpp>

namespace qf::timeseries {

class TimeSeries final {
public:
    TimeSeries() = default;
    explicit TimeSeries(Count capacity) { reserve(capacity); }

    [[nodiscard]] Count size() const noexcept { return values_.size(); }
    [[nodiscard]] Count capacity() const noexcept { return values_.capacity(); }
    [[nodiscard]] bool empty() const noexcept { return values_.empty(); }

    void reserve(Count capacity) {
        timestamps_.reserve(capacity);
        values_.reserve(capacity);
    }

    [[nodiscard]] ConstSpan<Timestamp> timestamps() const noexcept { return timestamps_; }
    [[nodiscard]] ConstSpan<Real> values() const noexcept { return values_; }
    [[nodiscard]] Timestamp timestamp(Index i) const noexcept { assert(i < size()); return timestamps_[i]; }
    [[nodiscard]] Real value(Index i) const noexcept { assert(i < size()); return values_[i]; }

    [[nodiscard]] Status append(Timestamp timestamp, Real value) noexcept;

    void append_unchecked(Timestamp timestamp, Real value) noexcept {
        timestamps_.push_back(timestamp);
        values_.push_back(value);
    }

    void clear() noexcept {
        timestamps_.clear();
        values_.clear();
    }

private:
    std::vector<Timestamp> timestamps_;
    std::vector<Real> values_;
};

} // namespace qf::timeseries
