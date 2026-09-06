#include <qf/linalg/algorithms.hpp>

#include <algorithm>
#include <cmath>
#include <utility>
#include <vector>

namespace qf::linalg {
namespace {


} // namespace

Real dot_unchecked(ConstSpan<Real> a, ConstSpan<Real> b) noexcept {
    Real total = 0.0;
    Real compensation = 0.0;
    for (Index i = 0; i < a.size(); ++i) {
        const Real p = a[i] * b[i];
        const Real next = total + p;
        compensation += (std::abs(total) >= std::abs(p)) ? (total - next) + p : (p - next) + total;
        total = next;
    }
    return total + compensation;
}

Result<Real> dot(ConstSpan<Real> a, ConstSpan<Real> b) noexcept {
    if (a.size() != b.size()) return Status{ErrorCode::invalid_argument, "vector sizes must match"};
    if (a.empty()) return Status{ErrorCode::empty_input, "dot product requires non-empty vectors"};
    return dot_unchecked(a, b);
}

Result<Real> squared_norm(ConstSpan<Real> x) noexcept {
    auto r = dot(x, x);
    return r ? Result<Real>{r.value()} : Result<Real>{r.error()};
}

Real norm_unchecked(ConstSpan<Real> x) noexcept {
    Real scale = 0.0, ssq = 1.0;
    for (Real v : x) {
        const Real a = std::abs(v);
        if (a == 0.0) continue;
        if (scale < a) {
            const Real r = scale / a;
            ssq = 1.0 + ssq * r * r;
            scale = a;
        } else {
            const Real r = a / scale;
            ssq += r * r;
        }
    }
    return scale == 0.0 ? 0.0 : scale * std::sqrt(ssq);
}

Result<Real> norm(ConstSpan<Real> x) noexcept {
    if (x.empty()) return Status{ErrorCode::empty_input, "norm requires a non-empty vector"};
    for (Real v : x) if (!std::isfinite(v)) return Status{ErrorCode::non_finite_input, "norm input must be finite"};
    return norm_unchecked(x);
}

Status matvec_into(const Matrix<Real>& A, ConstSpan<Real> x, Span<Real> y) noexcept {
    if (A.cols() != x.size() || A.rows() != y.size()) return Status{ErrorCode::invalid_argument, "matrix/vector dimensions must match"};
    for (Index i = 0; i < A.rows(); ++i) {
        const Real* row = A.data() + i * A.cols();
        Real total = 0.0;
        Real compensation = 0.0;
        for (Index j = 0; j < A.cols(); ++j) {
            const Real p = row[j] * x[j];
            const Real next = total + p;
            compensation += (std::abs(total) >= std::abs(p)) ? (total - next) + p : (p - next) + total;
            total = next;
        }
        y[i] = total + compensation;
    }
    return {};
}

Result<Vector<Real>> matvec(const Matrix<Real>& A, ConstSpan<Real> x) {
    if (A.cols() != x.size()) return Status{ErrorCode::invalid_argument, "matrix columns must match vector size"};
    Vector<Real> y(A.rows());
    auto status = matvec_into(A, x, y.span());
    if (!status) return status;
    return y;
}

Status matmul_into(const Matrix<Real>& A, const Matrix<Real>& B, MatrixView<Real> C) noexcept {
    if (A.cols() != B.rows() || C.rows() != A.rows() || C.cols() != B.cols())
        return Status{ErrorCode::invalid_argument, "matrix dimensions must match"};
    for (Index i = 0; i < A.rows(); ++i) {
        Real* crow = C.row(i).data();
        std::fill(crow, crow + C.cols(), 0.0);
        for (Index k = 0; k < A.cols(); ++k) {
            const Real a = A(i, k);
            const Real* brow = B.data() + k * B.cols();
            for (Index j = 0; j < B.cols(); ++j) crow[j] += a * brow[j];
        }
    }
    return {};
}

Result<Matrix<Real>> matmul(const Matrix<Real>& A, const Matrix<Real>& B) {
    if (A.cols() != B.rows()) return Status{ErrorCode::invalid_argument, "inner matrix dimensions must match"};
    Matrix<Real> C(A.rows(), B.cols());
    auto status = matmul_into(A, B, matrix_view(C));
    if (!status) return status;
    return C;
}

Result<Matrix<Real>> outer(ConstSpan<Real> a, ConstSpan<Real> b) {
    if (a.empty() || b.empty()) return Status{ErrorCode::empty_input, "outer product requires non-empty vectors"};
    Matrix<Real> M(a.size(), b.size());
    for (Index i = 0; i < a.size(); ++i) {
        Real* row = M.data() + i * b.size();
        const Real ai = a[i];
        for (Index j = 0; j < b.size(); ++j) row[j] = ai * b[j];
    }
    return M;
}

Result<Vector<Real>> solve_lower(const Matrix<Real>& L, ConstSpan<Real> b, Tolerance t) noexcept {
    if (L.rows() != L.cols() || L.rows() != b.size()) return Status{ErrorCode::invalid_argument, "lower solve requires square matrix matching rhs"};
    if (L.empty()) return Status{ErrorCode::empty_input, "lower solve requires non-empty matrix"};
    Vector<Real> x(b.size());
    for (Index i = 0; i < L.rows(); ++i) {
        Real s = b[i];
        for (Index j = 0; j < i; ++j) s -= L(i, j) * x[j];
        const Real d = L(i, i);
        if (!std::isfinite(d) || std::abs(d) <= t.absolute) return Status{ErrorCode::singular, "lower triangular matrix is singular"};
        x[i] = s / d;
    }
    return x;
}

Result<Vector<Real>> solve_upper(const Matrix<Real>& U, ConstSpan<Real> b, Tolerance t) noexcept {
    if (U.rows() != U.cols() || U.rows() != b.size()) return Status{ErrorCode::invalid_argument, "upper solve requires square matrix matching rhs"};
    if (U.empty()) return Status{ErrorCode::empty_input, "upper solve requires non-empty matrix"};
    Vector<Real> x(b.size());
    for (SignedIndex ii = static_cast<SignedIndex>(U.rows()) - 1; ii >= 0; --ii) {
        const Index i = static_cast<Index>(ii);
        Real s = b[i];
        for (Index j = i + 1; j < U.cols(); ++j) s -= U(i, j) * x[j];
        const Real d = U(i, i);
        if (!std::isfinite(d) || std::abs(d) <= t.absolute) return Status{ErrorCode::singular, "upper triangular matrix is singular"};
        x[i] = s / d;
    }
    return x;
}

Result<LUFactorization> lu(const Matrix<Real>& A, Tolerance t) {
    if (A.rows() != A.cols()) return Status{ErrorCode::invalid_argument, "LU requires a square matrix"};
    if (A.empty()) return Status{ErrorCode::empty_input, "LU requires a non-empty matrix"};
    LUFactorization f{A, Vector<Index>(A.rows()), 1};
    for (Index k = 0; k < A.rows(); ++k) {
        Index p = k;
        Real pivot = std::abs(f.lu(k, k));
        for (Index i = k + 1; i < A.rows(); ++i) {
            const Real candidate = std::abs(f.lu(i, k));
            if (candidate > pivot) { pivot = candidate; p = i; }
        }
        if (!std::isfinite(pivot) || pivot <= t.absolute) return Status{ErrorCode::singular, "matrix is singular or numerically singular"};
        f.pivots[k] = p;
        if (p != k) {
            Real* r1 = f.lu.data() + k * A.cols();
            Real* r2 = f.lu.data() + p * A.cols();
            for (Index j = 0; j < A.cols(); ++j) std::swap(r1[j], r2[j]);
            f.parity = -f.parity;
        }
        const Real diagonal = f.lu(k, k);
        for (Index i = k + 1; i < A.rows(); ++i) {
            Real* row = f.lu.data() + i * A.cols();
            const Real factor = row[k] / diagonal;
            row[k] = factor;
            const Real* pivot_row = f.lu.data() + k * A.cols();
            for (Index j = k + 1; j < A.cols(); ++j) row[j] -= factor * pivot_row[j];
        }
    }
    return f;
}

Result<Vector<Real>> solve(const LUFactorization& f, ConstSpan<Real> b, Tolerance t) noexcept {
    if (f.lu.rows() != b.size()) return Status{ErrorCode::invalid_argument, "rhs size must match factorization"};
    if (f.lu.empty()) return Status{ErrorCode::empty_input, "factorization is empty"};
    Vector<Real> pb(b);
    for (Index k = 0; k < b.size(); ++k) if (f.pivots[k] != k) std::swap(pb[k], pb[f.pivots[k]]);
    Vector<Real> y(b.size());
    for (Index i = 0; i < b.size(); ++i) {
        Real s = pb[i];
        for (Index j = 0; j < i; ++j) s -= f.lu(i, j) * y[j];
        y[i] = s;
    }
    return solve_upper(f.lu, y.span(), t);
}

Result<Vector<Real>> solve(const Matrix<Real>& A, ConstSpan<Real> b, Tolerance t) {
    auto f = lu(A, t);
    if (!f) return f.error();
    return solve(f.value(), b, t);
}

Result<Matrix<Real>> cholesky(const Matrix<Real>& A, Tolerance t) {
    if (A.rows() != A.cols()) return Status{ErrorCode::invalid_argument, "Cholesky requires a square matrix"};
    if (A.empty()) return Status{ErrorCode::empty_input, "Cholesky requires a non-empty matrix"};
    Matrix<Real> L(A.rows(), A.cols(), 0.0);
    for (Index i = 0; i < A.rows(); ++i) {
        for (Index j = 0; j <= i; ++j) {
            Real s = A(i, j);
            for (Index k = 0; k < j; ++k) s -= L(i, k) * L(j, k);
            if (i == j) {
                if (!std::isfinite(s) || s <= t.absolute) return Status{ErrorCode::numerical_failure, "matrix is not positive definite"};
                L(i, j) = std::sqrt(s);
            } else {
                L(i, j) = s / L(j, j);
            }
        }
    }
    return L;
}

Result<Vector<Real>> solve_spd(const Matrix<Real>& A, ConstSpan<Real> b, Tolerance t) {
    auto L = cholesky(A, t);
    if (!L) return L.error();
    auto y = solve_lower(L.value(), b, t);
    if (!y) return y.error();
    // Avoid constructing an explicit transpose matrix: solve L^T x = y directly.
    const Matrix<Real>& M = L.value();
    Vector<Real> x(b.size());
    for (SignedIndex ii = static_cast<SignedIndex>(M.rows()) - 1; ii >= 0; --ii) {
        const Index i = static_cast<Index>(ii);
        Real s = y.value()[i];
        for (Index j = i + 1; j < M.cols(); ++j) s -= M(j, i) * x[j];
        const Real d = M(i, i);
        if (!std::isfinite(d) || std::abs(d) <= t.absolute) return Status{ErrorCode::singular, "Cholesky diagonal is singular"};
        x[i] = s / d;
    }
    return x;
}

Result<QRFactorization> qr(const Matrix<Real>& A, Tolerance tolerance) {
    if (A.rows() == 0 || A.cols() == 0) return Status{ErrorCode::empty_input, "QR requires a non-empty matrix"};
    if (A.rows() < A.cols()) return Status{ErrorCode::invalid_argument, "thin QR requires rows >= columns"};
    const Index m = A.rows(), n = A.cols();

    // Compact Householder QR. The strict upper triangle is R; entries below
    // the diagonal store the reflector vectors. This avoids normal equations.
    Matrix<Real> R = A;
    std::vector<Real> tau(n, 0.0);
    for (Index k = 0; k < n; ++k) {
        Real scale = 0.0;
        for (Index i = k; i < m; ++i) scale = std::max(scale, std::abs(R(i, k)));
        if (!std::isfinite(scale) || scale <= tolerance.absolute)
            return Status{ErrorCode::singular, "matrix has numerically dependent columns"};

        Real ssq = 0.0;
        for (Index i = k; i < m; ++i) {
            const Real z = R(i, k) / scale;
            ssq += z * z;
        }
        const Real xnorm = scale * std::sqrt(ssq);
        const Real alpha = R(k, k) <= 0.0 ? xnorm : -xnorm;
        const Real v1 = R(k, k) - alpha;
        R(k, k) = alpha;
        for (Index i = k + 1; i < m; ++i) R(i, k) /= v1;

        Real vnorm2 = 1.0;
        for (Index i = k + 1; i < m; ++i) vnorm2 += R(i, k) * R(i, k);
        tau[k] = 2.0 / vnorm2;

        for (Index j = k + 1; j < n; ++j) {
            Real dotv = R(k, j);
            for (Index i = k + 1; i < m; ++i) dotv += R(i, k) * R(i, j);
            const Real beta = tau[k] * dotv;
            R(k, j) -= beta;
            for (Index i = k + 1; i < m; ++i) R(i, j) -= beta * R(i, k);
        }
    }

    // Form thin Q by applying H_{n-1} ... H_0 to the first n columns of I_m.
    Matrix<Real> Q(m, n, 0.0);
    for (Index j = 0; j < n; ++j) Q(j, j) = 1.0;
    for (SignedIndex kk = static_cast<SignedIndex>(n) - 1; kk >= 0; --kk) {
        const Index k = static_cast<Index>(kk);
        for (Index j = 0; j < n; ++j) {
            Real dotv = Q(k, j);
            for (Index i = k + 1; i < m; ++i) dotv += R(i, k) * Q(i, j);
            const Real beta = tau[k] * dotv;
            Q(k, j) -= beta;
            for (Index i = k + 1; i < m; ++i) Q(i, j) -= beta * R(i, k);
        }
    }

    Matrix<Real> Rthin(n, n, 0.0);
    for (Index i = 0; i < n; ++i)
        for (Index j = i; j < n; ++j)
            Rthin(i, j) = R(i, j);
    return QRFactorization{std::move(Q), std::move(Rthin)};
}

Result<Vector<Real>> apply_qt(const QRFactorization& f, ConstSpan<Real> b) noexcept {
    if (f.q.rows() != b.size()) return Status{ErrorCode::invalid_argument, "rhs size must match Q rows"};
    Vector<Real> y(f.q.cols());
    for (Index j = 0; j < f.q.cols(); ++j) {
        Real total = 0.0;
        for (Index i = 0; i < f.q.rows(); ++i) total += f.q(i, j) * b[i];
        y[j] = total;
    }
    return y;
}


Result<SymmetricEigenFactorization> symmetric_eigen(const Matrix<Real>& A, Tolerance tolerance) {
    if (A.rows() != A.cols())
        return Status{ErrorCode::invalid_argument, "symmetric eigendecomposition requires a square matrix"};
    if (A.empty())
        return Status{ErrorCode::empty_input, "symmetric eigendecomposition requires a non-empty matrix"};

    const Index n = A.rows();
    Matrix<Real> M = A;
    Matrix<Real> V(n, n, 0.0);
    for (Index i = 0; i < n; ++i) V(i, i) = 1.0;

    // Validate symmetry and finite input once. PCA depends on a symmetric
    // covariance matrix, so silently accepting asymmetric data is undesirable.
    Real scale = 0.0;
    for (Index i = 0; i < n; ++i) {
        for (Index j = i; j < n; ++j) {
            const Real a = M(i, j);
            const Real b = M(j, i);
            if (!std::isfinite(a) || !std::isfinite(b))
                return Status{ErrorCode::non_finite_input, "symmetric eigendecomposition received non-finite input"};
            scale = std::max(scale, std::max(std::abs(a), std::abs(b)));
            const Real symmetry_tol = tolerance.absolute + tolerance.relative *
                                      std::max({1.0, std::abs(a), std::abs(b)});
            if (std::abs(a - b) > symmetry_tol)
                return Status{ErrorCode::invalid_argument, "matrix is not symmetric"};
            const Real avg = 0.5 * (a + b);
            M(i, j) = avg;
            M(j, i) = avg;
        }
    }

    const Real convergence = tolerance.absolute + tolerance.relative * std::max(1.0, scale);
    const Count max_sweeps = std::max<Count>(32, 8 * n);
    bool converged = (n == 1);

    for (Count sweep = 0; sweep < max_sweeps && !converged; ++sweep) {
        Real max_offdiag = 0.0;
        for (Index pcol = 0; pcol < n; ++pcol)
            for (Index qcol = pcol + 1; qcol < n; ++qcol)
                max_offdiag = std::max(max_offdiag, std::abs(M(pcol, qcol)));

        if (max_offdiag <= convergence) {
            converged = true;
            break;
        }

        for (Index pcol = 0; pcol + 1 < n; ++pcol) {
            for (Index qcol = pcol + 1; qcol < n; ++qcol) {
                const Real apq = M(pcol, qcol);
                if (std::abs(apq) <= convergence) continue;

                const Real app = M(pcol, pcol);
                const Real aqq = M(qcol, qcol);
                const Real tau = (aqq - app) / (2.0 * apq);
                const Real sign_tau = tau >= 0.0 ? 1.0 : -1.0;
                const Real t = sign_tau / (std::abs(tau) + std::sqrt(1.0 + tau * tau));
                const Real c = 1.0 / std::sqrt(1.0 + t * t);
                const Real srot = t * c;

                M(pcol, pcol) = app - t * apq;
                M(qcol, qcol) = aqq + t * apq;
                M(pcol, qcol) = 0.0;
                M(qcol, pcol) = 0.0;

                for (Index k = 0; k < n; ++k) {
                    if (k == pcol || k == qcol) continue;
                    const Real mkp = M(k, pcol);
                    const Real mkq = M(k, qcol);
                    const Real new_kp = c * mkp - srot * mkq;
                    const Real new_kq = srot * mkp + c * mkq;
                    M(k, pcol) = new_kp;
                    M(pcol, k) = new_kp;
                    M(k, qcol) = new_kq;
                    M(qcol, k) = new_kq;
                }

                for (Index k = 0; k < n; ++k) {
                    const Real vkp = V(k, pcol);
                    const Real vkq = V(k, qcol);
                    V(k, pcol) = c * vkp - srot * vkq;
                    V(k, qcol) = srot * vkp + c * vkq;
                }
            }
        }
    }

    if (!converged)
        return Status{ErrorCode::numerical_failure, "symmetric eigendecomposition did not converge"};

    Vector<Real> eigenvalues(n);
    for (Index i = 0; i < n; ++i) eigenvalues[i] = M(i, i);

    // Sort descending by eigenvalue. PCA conventionally exposes the largest
    // variance directions first. Apply the same permutation to eigenvectors.
    for (Index i = 0; i < n; ++i) {
        Index best = i;
        for (Index j = i + 1; j < n; ++j)
            if (eigenvalues[j] > eigenvalues[best]) best = j;
        if (best != i) {
            std::swap(eigenvalues[i], eigenvalues[best]);
            for (Index r = 0; r < n; ++r) std::swap(V(r, i), V(r, best));
        }
    }
    return SymmetricEigenFactorization{std::move(eigenvalues), std::move(V)};
}

} // namespace qf::linalg
