#pragma once

#include <chrono>
#include <compare>
#include <cstdint>

#include <qf/core/types.hpp>

namespace qf {

class Duration final {
public:
    using rep = TimestampRep;
    using period = std::nano;
    using chrono_duration = std::chrono::duration<rep, period>;

    constexpr Duration() noexcept = default;
    explicit constexpr Duration(rep nanoseconds) noexcept : nanoseconds_(nanoseconds) {}

    [[nodiscard]] static constexpr Duration from_nanoseconds(rep value) noexcept { return Duration{value}; }
    [[nodiscard]] static constexpr Duration from_microseconds(rep value) noexcept {
        return Duration{value * 1'000};
    }
    [[nodiscard]] static constexpr Duration from_milliseconds(rep value) noexcept {
        return Duration{value * 1'000'000};
    }
    [[nodiscard]] static constexpr Duration from_seconds(rep value) noexcept {
        return Duration{value * 1'000'000'000};
    }

    [[nodiscard]] constexpr rep count() const noexcept { return nanoseconds_; }
    [[nodiscard]] constexpr double seconds() const noexcept {
        return static_cast<double>(nanoseconds_) * 1e-9;
    }

    constexpr auto operator<=>(const Duration&) const noexcept = default;
    constexpr Duration operator+(Duration other) const noexcept { return Duration{nanoseconds_ + other.nanoseconds_}; }
    constexpr Duration operator-(Duration other) const noexcept { return Duration{nanoseconds_ - other.nanoseconds_}; }

private:
    rep nanoseconds_ = 0;
};

class Timestamp final {
public:
    constexpr Timestamp() noexcept = default;
    explicit constexpr Timestamp(TimestampRep nanoseconds_since_epoch) noexcept
        : nanoseconds_since_epoch_(nanoseconds_since_epoch) {}

    [[nodiscard]] static constexpr Timestamp from_nanoseconds(TimestampRep value) noexcept {
        return Timestamp{value};
    }

    [[nodiscard]] constexpr TimestampRep count() const noexcept {
        return nanoseconds_since_epoch_;
    }

    [[nodiscard]] constexpr Timestamp operator+(Duration duration) const noexcept {
        return Timestamp{nanoseconds_since_epoch_ + duration.count()};
    }

    [[nodiscard]] constexpr Timestamp operator-(Duration duration) const noexcept {
        return Timestamp{nanoseconds_since_epoch_ - duration.count()};
    }

    [[nodiscard]] constexpr Duration operator-(Timestamp other) const noexcept {
        return Duration::from_nanoseconds(nanoseconds_since_epoch_ - other.nanoseconds_since_epoch_);
    }

    constexpr auto operator<=>(const Timestamp&) const noexcept = default;

private:
    TimestampRep nanoseconds_since_epoch_ = 0;
};

} // namespace qf
