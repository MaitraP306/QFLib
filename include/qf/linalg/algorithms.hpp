#pragma once

#include <qf/core/error.hpp>
#include <qf/core/numerical.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/linalg/matrix.hpp>
#include <qf/linalg/vector.hpp>

namespace qf::linalg {


[[nodiscard]] Result<Real> dot(ConstSpan<Real> a, ConstSpan<Real> b) noexcept;
[[nodiscard]] Real dot_unchecked(ConstSpan<Real> a, ConstSpan<Real> b) noexcept;
[[nodiscard]] Result<Real> squared_norm(ConstSpan<Real> x) noexcept;
[[nodiscard]] Result<Real> norm(ConstSpan<Real> x) noexcept;
[[nodiscard]] Real norm_unchecked(ConstSpan<Real> x) noexcept;

[[nodiscard]] Result<Vector<Real>> matvec(const Matrix<Real>& A, ConstSpan<Real> x);
[[nodiscard]] Status matvec_into(const Matrix<Real>& A, ConstSpan<Real> x, Span<Real> y) noexcept;

[[nodiscard]] Result<Matrix<Real>> matmul(const Matrix<Real>& A, const Matrix<Real>& B);
[[nodiscard]] Status matmul_into(const Matrix<Real>& A, const Matrix<Real>& B, MatrixView<Real> C) noexcept;

[[nodiscard]] Result<Matrix<Real>> outer(ConstSpan<Real> a, ConstSpan<Real> b);
[[nodiscard]] Result<Vector<Real>> solve_lower(const Matrix<Real>& L, ConstSpan<Real> b, Tolerance t = {}) noexcept;
[[nodiscard]] Result<Vector<Real>> solve_upper(const Matrix<Real>& U, ConstSpan<Real> b, Tolerance t = {}) noexcept;

struct LUFactorization final {
    Matrix<Real> lu;
    Vector<Index> pivots;
    int parity = 1;
};

[[nodiscard]] Result<LUFactorization> lu(const Matrix<Real>& A, Tolerance t = {});
[[nodiscard]] Result<Vector<Real>> solve(const LUFactorization& f, ConstSpan<Real> b, Tolerance t = {}) noexcept;
[[nodiscard]] Result<Vector<Real>> solve(const Matrix<Real>& A, ConstSpan<Real> b, Tolerance t = {});

[[nodiscard]] Result<Matrix<Real>> cholesky(const Matrix<Real>& A, Tolerance t = {});
[[nodiscard]] Result<Vector<Real>> solve_spd(const Matrix<Real>& A, ConstSpan<Real> b, Tolerance t = {});

struct QRFactorization final {
    Matrix<Real> q;
    Matrix<Real> r;
};

[[nodiscard]] Result<QRFactorization> qr(const Matrix<Real>& A, Tolerance tolerance = {});
[[nodiscard]] Result<Vector<Real>> apply_qt(const QRFactorization& f, ConstSpan<Real> b) noexcept;

struct SymmetricEigenFactorization final {
    Vector<Real> eigenvalues;
    Matrix<Real> eigenvectors;
};

[[nodiscard]] Result<SymmetricEigenFactorization> symmetric_eigen(
    const Matrix<Real>& A, Tolerance tolerance = {});

} // namespace qf::linalg
