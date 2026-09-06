#pragma once

#include <span>
#include <type_traits>

#include <qf/core/types.hpp>

namespace qf {

template <typename T, std::size_t Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

template <typename T>
using ConstSpan = std::span<const T>;

template <typename T>
[[nodiscard]] constexpr Span<T> make_span(T* data, Count size) noexcept {
    return Span<T>{data, size};
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr Span<T, N> make_span(T (&data)[N]) noexcept {
    return Span<T, N>{data};
}

template <typename T, std::size_t N>
[[nodiscard]] constexpr ConstSpan<T> make_const_span(const T (&data)[N]) noexcept {
    return ConstSpan<T>{data};
}

} // namespace qf
