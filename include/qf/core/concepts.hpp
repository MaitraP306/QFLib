#pragma once

#include <concepts>
#include <type_traits>

namespace qf {

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept FloatingPoint = std::floating_point<T>;

template <typename T>
concept Integral = std::integral<T>;

template <typename T>
concept SignedArithmetic = Arithmetic<T> && std::is_signed_v<T>;

template <typename T>
concept UnsignedIntegral = std::integral<T> && std::is_unsigned_v<T>;

} // namespace qf
