#include <qf/timeseries/series.hpp>

#include <cmath>

namespace qf::timeseries {

Status TimeSeries::append(Timestamp timestamp, Real value) noexcept {
    if (!std::isfinite(value)) {
        return Status{ErrorCode::non_finite_input, "TimeSeries received a non-finite observation"};
    }
    if (!timestamps_.empty() && timestamp < timestamps_.back()) {
        return Status{ErrorCode::invalid_argument, "TimeSeries timestamps must be nondecreasing"};
    }
    timestamps_.push_back(timestamp);
    values_.push_back(value);
    return {};
}

} // namespace qf::timeseries
