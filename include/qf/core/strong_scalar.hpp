#pragma once

#include <concepts>
#include <compare>
#include <qf/core/types.hpp>

namespace qf::detail {

template <typename Tag, bool Additive = true, typename ValueType = Real>
class StrongScalar final {
public:
    using value_type = ValueType;

    constexpr StrongScalar() noexcept = default;

    explicit constexpr StrongScalar(auto value) noexcept : value_(static_cast<ValueType>(value)) {}

    [[nodiscard]] constexpr ValueType value() const noexcept {
        return value_;
    }

    constexpr auto operator<=>(const StrongScalar&) const noexcept = default;

    constexpr StrongScalar operator+(const StrongScalar& other) const noexcept requires Additive{
        return StrongScalar{value_ + other.value_};
    }

    constexpr StrongScalar operator-(const StrongScalar& other) const noexcept requires Additive{
        return StrongScalar{value_ - other.value_};
    }

    constexpr StrongScalar& operator+=(const StrongScalar& other) noexcept requires Additive{
        value_ += other.value_;
        return *this;
    }

    constexpr StrongScalar& operator-=(const StrongScalar& other) noexcept requires Additive{
        value_ -= other.value_;
        return *this;
    }

    constexpr StrongScalar operator-() const noexcept requires Additive {
        return StrongScalar{-value_};
    }

    // need stronger formulation
    constexpr StrongScalar operator*(Real scalar) const noexcept requires Additive {
        return StrongScalar{value_ * scalar};
    }

    constexpr StrongScalar operator*(Int scalar) const noexcept requires Additive {
        return StrongScalar{value_ * scalar};
    }

    constexpr StrongScalar operator/(Real scalar) const noexcept requires std::same_as<ValueType, Real> {
        return StrongScalar{value_ / scalar};
    }
    constexpr bool operator>(Int scalar) const noexcept {
        return value_>scalar;
    }

    constexpr bool operator>=(Int scalar) const noexcept {
        return value_>=scalar;
    }

    constexpr bool operator<(Int scalar) const noexcept {
        return value_<scalar;
    }
    constexpr bool operator<=(Int scalar) const noexcept {
        return value_<=scalar;
    }
    constexpr bool operator==(Int scalar) const noexcept {
        return value_==scalar;
    }
    constexpr bool operator==(StrongScalar scalar) const noexcept {
        return value_==scalar.value();
    }
    constexpr bool operator>(Real scalar) const noexcept {
        return value_>scalar;
    }

    constexpr bool operator>=(Real scalar) const noexcept {
        return value_>=scalar;
    }

    constexpr bool operator<(Real scalar) const noexcept {
        return value_<scalar;
    }
    constexpr bool operator<=(Real scalar) const noexcept {
        return value_<=scalar;
    }
    constexpr bool operator==(Real scalar) const noexcept {
        return value_==scalar;
    }
    constexpr StrongScalar abs() const noexcept {
        return StrongScalar{value_<0? -value_ : value_};
    }
    constexpr StrongScalar min(StrongScalar s) const noexcept {
        return StrongScalar{value_<=s.value()? value_ : s.value()};
    }
    constexpr StrongScalar max(StrongScalar s) const noexcept {
        return StrongScalar{value_>=s.value()? value_ : s.value()};
    }
private:
    ValueType value_{};
};


template <typename T>
concept StrongScalarType = requires(T x) {
    typename T::value_type;
    { x.value() } noexcept -> std::same_as<typename T::value_type>;
};


template <typename T>
requires StrongScalarType<T> && std::same_as<typename T::value_type, Real>
[[nodiscard]] constexpr T operator*(Real scalar, T value) noexcept {
    return value * scalar;
}


template <typename T>
requires StrongScalarType<T>
[[nodiscard]] constexpr Real operator/(T lhs, T rhs) noexcept {
    return static_cast<Real>(lhs.value()) / static_cast<Real>(rhs.value());
}

} // namespace qf::detail