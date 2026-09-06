#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <span>
#include <vector>

#include <qf/core/span.hpp>
#include <qf/core/types.hpp>

namespace qf::linalg {

template <typename T = Real>
class Vector final {
public:
    using value_type = T;
    using container_type = std::vector<T>;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;

    Vector() = default;
    explicit Vector(Count size) : data_(size) {}
    Vector(Count size, const T& value) : data_(size, value) {}
    Vector(std::initializer_list<T> values) : data_(values) {}
    explicit Vector(ConstSpan<T> values) : data_(values.begin(), values.end()) {}

    [[nodiscard]] Count size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    [[nodiscard]] T* data() noexcept { return data_.data(); }
    [[nodiscard]] const T* data() const noexcept { return data_.data(); }

    [[nodiscard]] T& operator[](Index i) noexcept {
        assert(i < size());
        return data_[i];
    }
    [[nodiscard]] const T& operator[](Index i) const noexcept {
        assert(i < size());
        return data_[i];
    }

    [[nodiscard]] T& at(Index i) { return data_.at(i); }
    [[nodiscard]] const T& at(Index i) const { return data_.at(i); }

    [[nodiscard]] iterator begin() noexcept { return data_.begin(); }
    [[nodiscard]] iterator end() noexcept { return data_.end(); }
    [[nodiscard]] const_iterator begin() const noexcept { return data_.begin(); }
    [[nodiscard]] const_iterator end() const noexcept { return data_.end(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return data_.cbegin(); }
    [[nodiscard]] const_iterator cend() const noexcept { return data_.cend(); }

    [[nodiscard]] Span<T> span() noexcept { return Span<T>{data_}; }
    [[nodiscard]] ConstSpan<T> span() const noexcept { return ConstSpan<T>{data_}; }

    void fill(const T& value) { std::fill(data_.begin(), data_.end(), value); }

private:
    container_type data_;
};

template <typename T>
[[nodiscard]] constexpr Span<T> vector_view(Vector<T>& vector) noexcept {
    return vector.span();
}

template <typename T>
[[nodiscard]] constexpr ConstSpan<T> vector_view(const Vector<T>& vector) noexcept {
    return vector.span();
}

} // namespace qf::linalg
