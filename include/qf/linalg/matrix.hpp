#pragma once

#include <algorithm>
#include <cassert>
#include <initializer_list>
#include <span>
#include <stdexcept>
#include <vector>

#include <qf/core/span.hpp>
#include <qf/core/types.hpp>

namespace qf::linalg {

template <typename T = Real>
class Matrix final {
public:
    using value_type = T;
    using container_type = std::vector<T>;

    Matrix() = default;
    Matrix(Count rows, Count cols) : rows_(rows), cols_(cols), data_(rows * cols) {}
    Matrix(Count rows, Count cols, const T& value) : rows_(rows), cols_(cols), data_(rows * cols, value) {}

    Matrix(std::initializer_list<std::initializer_list<T>> values) {
        rows_ = values.size();
        cols_ = rows_ == 0 ? 0 : values.begin()->size();
        data_.reserve(rows_ * cols_);
        for (const auto& row : values) {
            assert(row.size() == cols_);
            data_.insert(data_.end(), row.begin(), row.end());
        }
    }

    [[nodiscard]] Count rows() const noexcept { return rows_; }
    [[nodiscard]] Count cols() const noexcept { return cols_; }
    [[nodiscard]] Count size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }
    [[nodiscard]] T* data() noexcept { return data_.data(); }
    [[nodiscard]] const T* data() const noexcept { return data_.data(); }

    [[nodiscard]] T& operator()(Index row, Index col) noexcept {
        assert(row < rows_ && col < cols_);
        return data_[row * cols_ + col];
    }
    [[nodiscard]] const T& operator()(Index row, Index col) const noexcept {
        assert(row < rows_ && col < cols_);
        return data_[row * cols_ + col];
    }

    [[nodiscard]] T& at(Index row, Index col) {
        if (row >= rows_ || col >= cols_) {
            throw std::out_of_range("qf::linalg::Matrix index out of range");
        }
        return data_[row * cols_ + col];
    }
    [[nodiscard]] const T& at(Index row, Index col) const {
        if (row >= rows_ || col >= cols_) {
            throw std::out_of_range("qf::linalg::Matrix index out of range");
        }
        return data_[row * cols_ + col];
    }

    [[nodiscard]] Span<T> span() noexcept { return Span<T>{data_}; }
    [[nodiscard]] ConstSpan<T> span() const noexcept { return ConstSpan<T>{data_}; }

    [[nodiscard]] Span<T> row(Index row) noexcept {
        assert(row < rows_);
        return Span<T>{data_.data() + row * cols_, cols_};
    }
    [[nodiscard]] ConstSpan<T> row(Index row) const noexcept {
        assert(row < rows_);
        return ConstSpan<T>{data_.data() + row * cols_, cols_};
    }

    void fill(const T& value) { std::fill(data_.begin(), data_.end(), value); }

private:
    Count rows_ = 0;
    Count cols_ = 0;
    container_type data_;
};

template <typename T>
class MatrixView final {
public:
    using value_type = T;

    constexpr MatrixView() noexcept = default;
    constexpr MatrixView(T* data, Count rows, Count cols, Count stride) noexcept
        : data_(data), rows_(rows), cols_(cols), stride_(stride) {}

    [[nodiscard]] constexpr Count rows() const noexcept { return rows_; }
    [[nodiscard]] constexpr Count cols() const noexcept { return cols_; }
    [[nodiscard]] constexpr T* data() const noexcept { return data_; }

    [[nodiscard]] T& operator()(Index row, Index col) const noexcept {
        assert(row < rows_ && col < cols_);
        return data_[row * stride_ + col];
    }

    [[nodiscard]] Span<T> row(Index row) const noexcept {
        assert(row < rows_);
        return Span<T>{data_ + row * stride_, cols_};
    }

private:
    T* data_ = nullptr;
    Count rows_ = 0;
    Count cols_ = 0;
    Count stride_ = 0;
};

template <typename T>
[[nodiscard]] MatrixView<T> matrix_view(Matrix<T>& matrix) noexcept {
    return MatrixView<T>{matrix.data(), matrix.rows(), matrix.cols(), matrix.cols()};
}

template <typename T>
[[nodiscard]] MatrixView<const T> matrix_view(const Matrix<T>& matrix) noexcept {
    return MatrixView<const T>{matrix.data(), matrix.rows(), matrix.cols(), matrix.cols()};
}

template <typename T>
[[nodiscard]] MatrixView<T> submatrix(MatrixView<T> matrix, Index row, Index col,
                                      Count rows, Count cols) noexcept {
    assert(row + rows <= matrix.rows() && col + cols <= matrix.cols());
    return MatrixView<T>{matrix.data() + row * matrix.cols() + col,
                         rows, cols, matrix.cols()};
}

template <typename T>
[[nodiscard]] MatrixView<const T> submatrix(MatrixView<const T> matrix, Index row, Index col,
                                            Count rows, Count cols) noexcept {
    assert(row + rows <= matrix.rows() && col + cols <= matrix.cols());
    return MatrixView<const T>{matrix.data() + row * matrix.cols() + col,
                               rows, cols, matrix.cols()};
}

} // namespace qf::linalg
