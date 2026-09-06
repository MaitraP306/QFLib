#pragma once

#include <span>

#include <qf/core/error.hpp>
#include <qf/core/types.hpp>
#include <qf/linalg/matrix.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::linalg {

template <typename T, std::size_t LhsExtent, std::size_t RhsExtent>
[[nodiscard]] Result<Vector<T>> add(std::span<T, LhsExtent> lhs, std::span<T, RhsExtent> rhs) {
    if (lhs.size() != rhs.size()) {
        return Status{ErrorCode::invalid_argument, "vector sizes must match"};
    }
    Vector<T> result(lhs.size());
    for (Index i = 0; i < lhs.size(); ++i) result[i] = lhs[i] + rhs[i];
    return result;
}

template <typename T, std::size_t LhsExtent, std::size_t RhsExtent>
[[nodiscard]] Result<Vector<T>> subtract(std::span<T, LhsExtent> lhs, std::span<T, RhsExtent> rhs) {
    if (lhs.size() != rhs.size()) {
        return Status{ErrorCode::invalid_argument, "vector sizes must match"};
    }
    Vector<T> result(lhs.size());
    for (Index i = 0; i < lhs.size(); ++i) result[i] = lhs[i] - rhs[i];
    return result;
}

template <typename T>
[[nodiscard]] Vector<T> scale(ConstSpan<T> values, T scalar) {
    Vector<T> result(values.size());
    for (Index i = 0; i < values.size(); ++i) result[i] = values[i] * scalar;
    return result;
}

template <typename T>
[[nodiscard]] Result<Matrix<T>> add(const Matrix<T>& lhs, const Matrix<T>& rhs) {
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols()) {
        return Status{ErrorCode::invalid_argument, "matrix shapes must match"};
    }
    Matrix<T> result(lhs.rows(), lhs.cols());
    for (Index i = 0; i < lhs.size(); ++i) result.data()[i] = lhs.data()[i] + rhs.data()[i];
    return result;
}

template <typename T>
[[nodiscard]] Result<Matrix<T>> subtract(const Matrix<T>& lhs, const Matrix<T>& rhs) {
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols()) {
        return Status{ErrorCode::invalid_argument, "matrix shapes must match"};
    }
    Matrix<T> result(lhs.rows(), lhs.cols());
    for (Index i = 0; i < lhs.size(); ++i) result.data()[i] = lhs.data()[i] - rhs.data()[i];
    return result;
}

template <typename T>
[[nodiscard]] Matrix<T> scale(const Matrix<T>& values, T scalar) {
    Matrix<T> result(values.rows(), values.cols());
    for (Index i = 0; i < values.size(); ++i) result.data()[i] = values.data()[i] * scalar;
    return result;
}

template <typename T>
[[nodiscard]] Matrix<T> transpose(const Matrix<T>& matrix) {
    Matrix<T> result(matrix.cols(), matrix.rows());
    for (Index row = 0; row < matrix.rows(); ++row) {
        for (Index col = 0; col < matrix.cols(); ++col) {
            result(col, row) = matrix(row, col);
        }
    }
    return result;
}

} // namespace qf::linalg
