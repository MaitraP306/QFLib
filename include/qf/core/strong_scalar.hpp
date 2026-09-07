#pragma once

#include <compare>
#include <concepts>
#include <type_traits>

#include <qf/core/types.hpp>

namespace qf::detail {

template <typename Tag, bool Additive = true>
class StrongScalar final {
public:
    using value_type = Real;

    constexpr StrongScalar() noexcept = default;
    explicit constexpr StrongScalar(Real value) noexcept : value_(value) {}

    [[nodiscard]] constexpr Real value() const noexcept { return value_; }
    constexpr auto operator<=>(const StrongScalar&) const noexcept = default;
    constexpr StrongScalar operator+(const StrongScalar& other) const noexcept requires Additive { return StrongScalar{value_ + other.value_}; }
    constexpr StrongScalar operator-(const StrongScalar& other) const noexcept requires Additive { return StrongScalar{value_ - other.value_}; }
    constexpr StrongScalar& operator+=(const StrongScalar& other) noexcept requires Additive { value_ += other.value_; return *this; }
    constexpr StrongScalar& operator-=(const StrongScalar& other) noexcept requires Additive { value_ -= other.value_; return *this; }
    constexpr StrongScalar operator-() const noexcept requires Additive { return StrongScalar{-value_}; }

    constexpr StrongScalar operator*(Real scalar) const noexcept {
        return StrongScalar{value_ * scalar};
    }
    constexpr StrongScalar operator/(Real scalar) const noexcept {
        return StrongScalar{value_ / scalar};
    }

private:
    Real value_ = 0.0;
};

template <typename T>
concept StrongScalarType = requires(T x) {
    typename T::value_type;
    { x.value() } noexcept -> std::same_as<Real>;
};

template <typename T>
requires StrongScalarType<T>
[[nodiscard]] constexpr T operator*(Real scalar, T value) noexcept {
    return value * scalar;
}

template <typename T>
requires StrongScalarType<T>
[[nodiscard]] constexpr Real operator/(T lhs, T rhs) noexcept {
    return lhs.value() / rhs.value();
}

} // namespace qf::detail
