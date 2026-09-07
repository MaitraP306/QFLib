# QFLib

QFLib is a C++20 numerical and quantitative-finance library designed to
sit underneath a strategy-independent research and trading stack.

------------------------------------------------------------------------

# Installation and build

## Requirements

-   C++20 compiler
-   CMake 3.20 or newer
-   A standard C++20 library

No external numerical dependency is required by the current
implementation.

## Configure and build

``` bash
cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure
```

For a release build:

``` bash
cmake --preset release
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
```

The exact preset names should be checked against `CMakePresets.json` in
the repository if you add or change build configurations.

## Useful CMake options

``` text
QFLIB_BUILD_TESTS          Build tests. Default: ON
QFLIB_BUILD_BENCHMARKS     Build benchmarks. Default: OFF
QFLIB_BUILD_EXAMPLES       Build examples. Default: ON
QFLIB_ENABLE_SANITIZERS    Enable ASan/UBSan. Default: OFF
QFLIB_ENABLE_LTO           Enable LTO/IPO in Release when supported. Default: ON
QFLIB_ENABLE_NATIVE_ARCH   Enable -march=native/-mtune=native. Default: OFF
```

For example:

``` bash
cmake -S . -B build/release \
  -DCMAKE_BUILD_TYPE=Release \
  -DQFLIB_BUILD_TESTS=ON \
  -DQFLIB_ENABLE_LTO=ON

cmake --build build/release -j
ctest --test-dir build/release --output-on-failure
```

## CMake integration

QFLib exports the target:

``` cmake
find_package(QFLib CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE qf::qflib)
```

Headers are installed under `include/qf`.

------------------------------------------------------------------------

# Include hierarchy

The library can be consumed through the umbrella header:

``` cpp
#include <qf/qf.hpp>
```

Or through narrower module headers:

``` cpp
#include <qf/core.hpp>
#include <qf/math.hpp>
#include <qf/linalg.hpp>
#include <qf/stats.hpp>
#include <qf/timeseries.hpp>
#include <qf/probability.hpp>
#include <qf/stochastic.hpp>
#include <qf/optimization.hpp>
#include <qf/finance.hpp>
#include <qf/portfolio.hpp>
#include <qf/risk.hpp>
#include <qf/pricing.hpp>
#include <qf/market.hpp>
```

For library implementation code, prefer the narrowest header that
expresses the dependency. For application code, the umbrella headers are
convenient.

------------------------------------------------------------------------

# API reference

The sections below document the public API currently exposed by
`include/qf`.

------------------------------------------------------------------------

# 1. Core

Namespace: `qf`

Core defines the vocabulary used by the rest of the library. It
deliberately contains no pricing model, strategy, portfolio logic, or
market-data protocol.

## 1.1 Fundamental types

Header: `qf/core/types.hpp`

``` cpp
using Real = double;
using Index = std::size_t;
using SignedIndex = std::ptrdiff_t;
using Count = std::size_t;
using TimestampRep = std::int64_t;
inline constexpr Index npos = std::numeric_limits<Index>::max();
```

### `Real`

The canonical floating-point scalar used by QFLib.

### `Index`

Unsigned index type used for positions into QFLib containers.

### `SignedIndex`

Signed index type for calculations where negative offsets are useful.

### `Count`

Semantic alias for sizes/counts.

### `TimestampRep`

Signed 64-bit representation used internally by `Timestamp` and
`Duration`.

### `npos`

Sentinel representing an invalid/not-found index.

------------------------------------------------------------------------

## 1.2 Concepts

Header: `qf/core/concepts.hpp`

``` cpp
qf::Arithmetic<T>
qf::FloatingPoint<T>
qf::Integral<T>
qf::SignedArithmetic<T>
qf::UnsignedIntegral<T>
```

These are small C++20 concepts used to constrain generic numerical code.

Example:

``` cpp
template <qf::FloatingPoint T>
T kernel(T x) {
    return x * x;
}
```

------------------------------------------------------------------------

## 1.3 Mathematical constants

Header: `qf/core/constants.hpp`

All constants are `inline constexpr Real` values and are header-only.

``` cpp
qf::constants::pi
qf::constants::two_pi
qf::constants::half_pi
qf::constants::e
qf::constants::sqrt_two
qf::constants::sqrt_two_pi
qf::constants::machine_epsilon
qf::constants::infinity
qf::constants::quiet_nan
```

Example:

``` cpp
Real angle = qf::constants::half_pi;
```

`machine_epsilon`, `infinity`, and `quiet_nan` are obtained from
`std::numeric_limits<Real>`.

------------------------------------------------------------------------

## 1.4 Numerical utilities

Header: `qf/core/numerical.hpp`

### `Tolerance`

``` cpp
struct Tolerance {
    Real absolute = 1e-12;
    Real relative = 1e-10;
};
```

Represents mixed absolute/relative comparison tolerances.

### `valid_tolerance`

``` cpp
bool valid_tolerance(Tolerance t) noexcept;
```

Checks that both tolerances are finite and non-negative.

### `is_finite`

``` cpp
bool is_finite(Real x) noexcept;
```

### `is_nan`

``` cpp
bool is_nan(Real x) noexcept;
```

### `is_infinite`

``` cpp
bool is_infinite(Real x) noexcept;
```

### `almost_equal`

``` cpp
bool almost_equal(Real a, Real b, Tolerance tolerance = {}) noexcept;
```

Uses the criterion

``` text
|a - b| <= max(abs_tol, rel_tol * max(|a|, |b|))
```

Exact equality is accepted first. Non-finite unequal values are
rejected.

### `clamp`

``` cpp
Real clamp(Real x, Real lo, Real hi) noexcept;
```

Returns `lo` when `x < lo`, `hi` when `x > hi`, otherwise `x`.

------------------------------------------------------------------------

## 1.5 Spans

Header: `qf/core/span.hpp`

QFLib aliases `std::span` rather than introducing another storage
abstraction.

``` cpp
template <typename T, std::size_t Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

template <typename T>
using ConstSpan = std::span<const T>;
```

### `make_span`

``` cpp
Span<T> make_span(T* data, Count size) noexcept;
Span<T, N> make_span(T (&data)[N]) noexcept;
```

### `make_const_span`

``` cpp
ConstSpan<T> make_const_span(const T (&data)[N]) noexcept;
```

A span is non-owning. It does not allocate or copy the referenced data.

------------------------------------------------------------------------

## 1.6 Error handling

Header: `qf/core/error.hpp`

### `ErrorCode`

``` cpp
enum class ErrorCode {
    none,
    invalid_argument,
    out_of_range,
    empty_input,
    singular,
    non_finite_input,
    numerical_failure,
    not_implemented,
};
```

### `Status`

Represents success or an error without allocating an error message.

``` cpp
Status();
Status(ErrorCode code, std::string_view message);

bool ok() const noexcept;
ErrorCode code() const noexcept;
std::string_view message() const noexcept;
explicit operator bool() const noexcept;
```

Example:

``` cpp
Status status = do_work();
if (!status) {
    // inspect status.code() / status.message()
}
```

### `Result<T>`

Stores either a value or a `Status`.

``` cpp
Result(const T& value);
Result(T&& value);
Result(Status error);

bool has_value() const noexcept;
explicit operator bool() const noexcept;
const T& value() const &;
T& value() &;
T&& value() &&;
Status error() const noexcept;
```

Example:

``` cpp
Result<Real> result = qf::stats::mean(values);
if (!result) {
    std::cerr << result.error().message() << '\n';
    return;
}
Real m = result.value();
```

`value()` assumes the result contains a value. Check the result first.

### `success`

``` cpp
Status success() noexcept;
```

Convenience function for returning a successful `Status`.

------------------------------------------------------------------------

## 1.7 Time types

Header: `qf/core/time.hpp`

QFLib uses integer nanoseconds internally.

### `Duration`

``` cpp
class Duration;
```

Construction:

``` cpp
Duration::from_nanoseconds(rep value)
Duration::from_microseconds(rep value)
Duration::from_milliseconds(rep value)
Duration::from_seconds(rep value)
```

Accessors:

``` cpp
rep count() const noexcept;
Real seconds() const noexcept;
```

Arithmetic:

``` cpp
Duration + Duration
Duration - Duration
```

Comparison operators are available through C++20 three-way comparison.

Example:

``` cpp
Duration dt = Duration::from_milliseconds(10);
Real seconds = dt.seconds();
```

### `Timestamp`

Represents a point in time as nanoseconds since epoch.

``` cpp
Timestamp::from_nanoseconds(TimestampRep value)
TimestampRep count() const noexcept;
```

Arithmetic:

``` cpp
Timestamp + Duration -> Timestamp
Timestamp - Duration -> Timestamp
Timestamp - Timestamp -> Duration
```

Example:

``` cpp
Timestamp t0 = Timestamp::from_nanoseconds(1'000'000'000);
Timestamp t1 = t0 + Duration::from_seconds(5);
Duration dt = t1 - t0;
```

Financial calendars and day-count conventions are intentionally not part
of core.

------------------------------------------------------------------------

## 1.8 Strong scalar primitive

Header: `qf/core/strong_scalar.hpp`

`qf::detail::StrongScalar<Tag, Additive>` is the implementation
mechanism behind the financial scalar types.

The object stores exactly one `Real` value.

``` cpp
template <typename Tag, bool Additive = true>
class StrongScalar;
```

Core operations:

``` cpp
StrongScalar();
explicit StrongScalar(Real value);
Real value() const noexcept;
```

For additive types:

``` cpp
T + T -> T
T - T -> T
T += T
T -= T
-T -> T
```

All strong scalars support:

``` cpp
T * Real -> T
T / Real -> T
Real * T -> T
T / T -> Real
```

The type is intended to provide compile-time semantic separation without
adding per-object metadata.

`StrongScalarType<T>` is the concept used by generic strong-scalar
utilities.

------------------------------------------------------------------------

# 2. Math

Namespace: `qf::math`

Headers:

``` cpp
#include <qf/math.hpp>
```

## 2.1 Scalar operations

Header: `qf/math/scalar.hpp`

``` cpp
square(x)
cube(x)
sign(x)
lerp(a, b, t)
hypot2(x, y)
exp(x)
expm1(x)
log(x)
log1p(x)
sqrt(x)
cbrt(x)
sin(x)
cos(x)
tan(x)
asin(x)
acos(x)
atan(x)
atan2(y, x)
floor(x)
ceil(x)
round(x)
log1pexp(x)
sigmoid(x)
logit(p)
```

The elementary functions wrap the corresponding standard-library math
operations. `log1pexp` is a numerically stable implementation of

``` text
log(1 + exp(x))
```

and `sigmoid` implements

``` text
1 / (1 + exp(-x))
```

with stable branches for large positive/negative inputs.

`logit` implements

``` text
log(p / (1 - p))
```

for probabilities in the valid open interval.

### `square`

``` cpp
square(x)
```

Returns `x*x`.

### `cube`

``` cpp
cube(x)
```

Returns `x*x*x`.

### `sign`

Returns `-1`, `0`, or `1` according to the sign of the input.

### `lerp`

``` cpp
lerp(a, b, t)
```

Linear interpolation between `a` and `b`.

------------------------------------------------------------------------

## 2.2 Reductions

Header: `qf/math/reductions.hpp`

``` cpp
Real sum(ConstSpan<double> values) noexcept;
Real sum_squares(ConstSpan<double> values) noexcept;
Real min(ConstSpan<double> values) noexcept;
Real max(ConstSpan<double> values) noexcept;
Real mean(ConstSpan<double> values) noexcept;
Real rms(ConstSpan<double> values) noexcept;
```

The sum routines use compensated summation to reduce accumulation error.

These APIs currently expose `double` in their signatures rather than the
`Real` alias; this is one area that can be made more consistent if
`Real` is later generalized.

------------------------------------------------------------------------

## 2.3 Polynomial evaluation

Header: `qf/math/polynomial.hpp`

``` cpp
Real horner(Real x, ConstSpan<Real> coefficients) noexcept;
```

Evaluates a polynomial using Horner's method. If coefficients are
ordered from highest degree to constant term,

``` text
c0*x^n + c1*x^(n-1) + ... + cn
```

is evaluated as a linear sequence of multiply-add operations.

------------------------------------------------------------------------

# 3. Linear algebra

Namespace: `qf::linalg`

Headers:

``` cpp
#include <qf/linalg.hpp>
```

## 3.1 `Vector<T>`

Header: `qf/linalg/vector.hpp`

Owning contiguous vector backed by `std::vector<T>`.

Construction:

``` cpp
Vector();
Vector(Count size);
Vector(Count size, const T& value);
Vector(std::initializer_list<T> values);
explicit Vector(ConstSpan<T> values);
```

Accessors:

``` cpp
Count size() const noexcept;
bool empty() const noexcept;
T* data() noexcept;
const T* data() const noexcept;
T& operator[](Index i) noexcept;
const T& operator[](Index i) const noexcept;
T& at(Index i);
const T& at(Index i) const;
Span<T> span() noexcept;
ConstSpan<T> span() const noexcept;
```

Iteration:

``` cpp
begin(), end(), cbegin(), cend()
```

Mutation:

``` cpp
void fill(const T& value);
```

Views:

``` cpp
vector_view(Vector<T>&)
vector_view(const Vector<T>&)
```

`operator[]` is assertion-checked in debug builds. `at()` uses
bounds-checked access.

------------------------------------------------------------------------

## 3.2 `Matrix<T>`

Header: `qf/linalg/matrix.hpp`

Owning row-major dense matrix.

Construction:

``` cpp
Matrix();
Matrix(Count rows, Count cols);
Matrix(Count rows, Count cols, const T& value);
Matrix(std::initializer_list<std::initializer_list<T>> values);
```

Accessors:

``` cpp
Count rows() const noexcept;
Count cols() const noexcept;
Count size() const noexcept;
T* data() noexcept;
const T* data() const noexcept;
```

Element access:

``` cpp
matrix(row, col)
matrix.at(row, col)
```

Views:

``` cpp
Span<T> span() noexcept;
ConstSpan<T> span() const noexcept;
Span<T> row(Index row) noexcept;
ConstSpan<T> row(Index row) const noexcept;
```

`fill()` fills all elements.

------------------------------------------------------------------------

## 3.3 `MatrixView<T>`

A non-owning matrix view with an explicit row stride.

``` cpp
MatrixView(T* data, Count rows, Count cols, Count stride);
```

Accessors:

``` cpp
rows()
cols()
data()
operator()(row, col)
row(row)
```

Helper functions:

``` cpp
matrix_view(Matrix<T>&)
matrix_view(const Matrix<T>&)
submatrix(MatrixView<T>, row, col, rows, cols)
```

`MatrixView` is useful when an algorithm should write into caller-owned
matrix storage or operate on a submatrix without copying.

------------------------------------------------------------------------

## 3.4 Elementwise operations

Header: `qf/linalg/elementwise.hpp`

Vectors:

``` cpp
add(lhs, rhs)
subtract(lhs, rhs)
scale(values, scalar)
```

Matrices:

``` cpp
add(lhs, rhs)
subtract(lhs, rhs)
scale(matrix, scalar)
transpose(matrix)
```

Vector add/subtract require matching sizes. Matrix add/subtract require
matching shapes.

------------------------------------------------------------------------

## 3.5 Linear algebra algorithms

Header: `qf/linalg/algorithms.hpp`

### Dot product

``` cpp
Result<Real> dot(ConstSpan<Real> a, ConstSpan<Real> b) noexcept;
Real dot_unchecked(ConstSpan<Real> a, ConstSpan<Real> b) noexcept;
```

### Norms

``` cpp
Result<Real> squared_norm(ConstSpan<Real> x) noexcept;
Result<Real> norm(ConstSpan<Real> x) noexcept;
Real norm_unchecked(ConstSpan<Real> x) noexcept;
```

`norm(x)` is the Euclidean norm:

``` text
sqrt(sum(x_i^2))
```

### Matrix-vector multiplication

``` cpp
Result<Vector<Real>> matvec(const Matrix<Real>& A, ConstSpan<Real> x);
Status matvec_into(const Matrix<Real>& A, ConstSpan<Real> x, Span<Real> y) noexcept;
```

Computes `A*x`.

### Matrix multiplication

``` cpp
Result<Matrix<Real>> matmul(const Matrix<Real>& A, const Matrix<Real>& B);
Status matmul_into(const Matrix<Real>& A, const Matrix<Real>& B, MatrixView<Real> C) noexcept;
```

Computes `A*B`.

### Outer product

``` cpp
Result<Matrix<Real>> outer(ConstSpan<Real> a, ConstSpan<Real> b);
```

Produces the matrix

``` text
M_ij = a_i * b_j
```

### Triangular solves

``` cpp
Result<Vector<Real>> solve_lower(const Matrix<Real>& L, ConstSpan<Real> b, Tolerance t = {}) noexcept;
Result<Vector<Real>> solve_upper(const Matrix<Real>& U, ConstSpan<Real> b, Tolerance t = {}) noexcept;
```

Solve `Lx=b` or `Ux=b`.

### LU factorization

``` cpp
struct LUFactorization {
    Matrix<Real> lu;
    Vector<Index> pivots;
    int parity = 1;
};

Result<LUFactorization> lu(const Matrix<Real>& A, Tolerance t = {});
Result<Vector<Real>> solve(const LUFactorization& f, ConstSpan<Real> b, Tolerance t = {}) noexcept;
Result<Vector<Real>> solve(const Matrix<Real>& A, ConstSpan<Real> b, Tolerance t = {});
```

The factorization uses partial pivoting. `parity` records the
permutation parity.

### Cholesky

``` cpp
Result<Matrix<Real>> cholesky(const Matrix<Real>& A, Tolerance t = {});
Result<Vector<Real>> solve_spd(const Matrix<Real>& A, ConstSpan<Real> b, Tolerance t = {});
```

For symmetric positive-definite matrices, Cholesky provides a triangular
factorization suitable for efficient solves.

### QR factorization

``` cpp
struct QRFactorization {
    Matrix<Real> q;
    Matrix<Real> r;
};

Result<QRFactorization> qr(const Matrix<Real>& A, Tolerance tolerance = {});
Result<Vector<Real>> apply_qt(const QRFactorization& f, ConstSpan<Real> b) noexcept;
```

The implementation uses Householder transformations.

### Symmetric eigendecomposition

``` cpp
struct SymmetricEigenFactorization {
    Vector<Real> eigenvalues;
    Matrix<Real> eigenvectors;
};

Result<SymmetricEigenFactorization> symmetric_eigen(
    const Matrix<Real>& A,
    Tolerance tolerance = {});
```

Eigenvalues are returned in descending order, with corresponding
eigenvectors stored as columns.

------------------------------------------------------------------------

# 4. Statistics

Namespace: `qf::stats`

Header:

``` cpp
#include <qf/stats.hpp>
```

## 4.1 Descriptive statistics

Header: `qf/stats/descriptive.hpp`

``` cpp
struct DescriptiveStats {
    Real mean;
    Real variance;
    Real standard_deviation;
    Real minimum;
    Real maximum;
    Count count;
};
```

Functions:

``` cpp
Result<DescriptiveStats> describe(ConstSpan<Real> values) noexcept;
Result<Real> mean(ConstSpan<Real> values) noexcept;
Result<Real> variance(ConstSpan<Real> values) noexcept;
Result<Real> standard_deviation(ConstSpan<Real> values) noexcept;
```

The variance and standard deviation use the sample convention where
appropriate.

------------------------------------------------------------------------

## 4.2 Online moments

Header: `qf/stats/online.hpp`

`OnlineMoments` stores sufficient state for one-pass mean and variance
calculation.

Public state:

``` cpp
Count count;
Real mean;
Real m2;
```

Methods:

``` cpp
Status update(Real x) noexcept;
void update_unchecked(Real x) noexcept;
void reset() noexcept;
Status merge(const OnlineMoments& other) noexcept;
void merge_unchecked(const OnlineMoments& other) noexcept;
Real variance() const noexcept;
Real standard_deviation() const noexcept;
```

The update follows the Welford-style recurrence, avoiding the need to
store the complete sample history.

`merge()` allows independently accumulated states to be combined.

------------------------------------------------------------------------

## 4.3 Online covariance

``` cpp
struct OnlineCovariance {
    Count count;
    Real mean_x;
    Real mean_y;
    Real m2_x;
    Real m2_y;
    Real c;
};
```

Methods:

``` cpp
Status update(Real x, Real y) noexcept;
void update_unchecked(Real x, Real y) noexcept;
void reset() noexcept;
Real covariance() const noexcept;
Real variance_x() const noexcept;
Real variance_y() const noexcept;
Real correlation() const noexcept;
```

This supports one-pass covariance/correlation calculation without
storing paired observations.

------------------------------------------------------------------------

## 4.4 Online regression

`OnlineRegression` wraps `OnlineCovariance` and provides:

``` cpp
Status update(Real x, Real y) noexcept;
void update_unchecked(Real x, Real y) noexcept;
void reset() noexcept;
Real slope() const noexcept;
Real intercept() const noexcept;
Real r_squared() const noexcept;
```

For ordinary simple regression,

``` text
slope = covariance_numerator / x_variance_numerator
intercept = mean_y - slope * mean_x
R² = correlation²
```

------------------------------------------------------------------------

## 4.5 Exponentially weighted moments

``` cpp
struct EWMoments {
    Real alpha;
    Real mean;
    Real variance_state;
    bool initialized;
};
```

Construction:

``` cpp
explicit EWMoments(Real alpha_value) noexcept;
```

Methods:

``` cpp
bool valid() const noexcept;
Status update(Real x) noexcept;
void update_unchecked(Real x) noexcept;
void reset() noexcept;
Real variance() const noexcept;
Real standard_deviation() const noexcept;
```

`alpha` must satisfy:

``` text
0 < alpha <= 1
```

------------------------------------------------------------------------

## 4.6 Rolling moments

`RollingMoments` is a caller-owned, fixed-window online statistic.

``` cpp
explicit RollingMoments(Span<Real> storage) noexcept;
```

The caller supplies storage for the rolling window.

Methods:

``` cpp
bool valid() const noexcept;
Count window_size() const noexcept;
Count count() const noexcept;
bool full() const noexcept;
Status update(Real x) noexcept;
void update_unchecked(Real x) noexcept;
void reset() noexcept;
Real mean() const noexcept;
Real variance() const noexcept;
Real standard_deviation() const noexcept;
```

Updates are designed to run in O(1) time once the window is established.

------------------------------------------------------------------------

## 4.7 Covariance and correlation

Header: `qf/stats/covariance.hpp`

``` cpp
Result<Real> covariance(ConstSpan<Real> x, ConstSpan<Real> y) noexcept;
Result<Real> correlation(ConstSpan<Real> x, ConstSpan<Real> y) noexcept;
Result<linalg::Matrix<Real>> covariance_matrix(
    ConstSpan<Real> const* columns,
    Count column_count);
```

`covariance_matrix` treats the input as a collection of columns, with
each column represented by a `ConstSpan<Real>`.

------------------------------------------------------------------------

## 4.8 OLS

Header: `qf/stats/ols.hpp`

``` cpp
struct OLSResult {
    linalg::Vector<Real> coefficients;
    linalg::Vector<Real> residuals;
    Real residual_sum_of_squares;
    Real r_squared;
};

Result<OLSResult> ordinary_least_squares(
    const linalg::Matrix<Real>& x,
    ConstSpan<Real> y);
```

The implementation solves the least-squares problem using QR rather than
explicitly forming and inverting `X'X`.

The caller is responsible for deciding whether an intercept column
belongs in `x`.

------------------------------------------------------------------------

## 4.9 PCA

Header: `qf/stats/pca.hpp`

``` cpp
struct PCAResult {
    linalg::Vector<Real> mean;
    linalg::Vector<Real> eigenvalues;
    linalg::Vector<Real> explained_variance_ratio;
    linalg::Matrix<Real> components;
};
```

### `pca_fit`

``` cpp
Result<PCAResult> pca_fit(
    const linalg::Matrix<Real>& observations,
    Count n_components = 0,
    Tolerance tolerance = {});
```

Rows are observations and columns are features.

If `n_components` is zero, the full available component set is retained.

### `pca_transform`

``` cpp
Result<linalg::Matrix<Real>> pca_transform(
    const PCAResult& model,
    const linalg::Matrix<Real>& observations);
```

Projects observations into the fitted component space.

### `pca_fit_transform`

Fits and transforms in one call.

------------------------------------------------------------------------

# 5. Time series

Namespace: `qf::timeseries`

Headers:

``` cpp
#include <qf/timeseries.hpp>
```

## 5.1 `TimeSeries`

Header: `qf/timeseries/series.hpp`

`TimeSeries` stores timestamps and values as separate contiguous arrays.

Construction:

``` cpp
TimeSeries();
explicit TimeSeries(Count capacity);
```

Accessors:

``` cpp
Count size() const noexcept;
Count capacity() const noexcept;
bool empty() const noexcept;
ConstSpan<Timestamp> timestamps() const noexcept;
ConstSpan<Real> values() const noexcept;
Timestamp timestamp(Index i) const noexcept;
Real value(Index i) const noexcept;
```

Mutation:

``` cpp
void reserve(Count capacity);
Status append(Timestamp timestamp, Real value);
void append_unchecked(Timestamp timestamp, Real value);
void clear();
```

The checked `append` path requires finite values and non-decreasing
timestamps.

The storage is structure-of-arrays rather than an array of
`{timestamp,value}` structures, which makes value-oriented numerical
scans straightforward.

------------------------------------------------------------------------

## 5.2 Batch operations

Header: `qf/timeseries/operations.hpp`

### `difference`

``` cpp
Result<linalg::Vector<Real>> difference(
    ConstSpan<Real> values,
    Count lag = 1) noexcept;
```

Computes:

``` text
x[t] - x[t-lag]
```

### `simple_returns`

``` cpp
Result<linalg::Vector<Real>> simple_returns(
    ConstSpan<Real> prices,
    Count lag = 1) noexcept;
```

Computes:

``` text
P[t] / P[t-lag] - 1
```

### `log_returns`

``` cpp
Result<linalg::Vector<Real>> log_returns(
    ConstSpan<Real> prices,
    Count lag = 1) noexcept;
```

Computes logarithmic price returns.

### `rolling_mean`

``` cpp
Result<linalg::Vector<Real>> rolling_mean(
    ConstSpan<Real> values,
    Count window) noexcept;
```

Computes a fixed-size rolling arithmetic mean.

### `rolling_standard_deviation`

``` cpp
Result<linalg::Vector<Real>> rolling_standard_deviation(
    ConstSpan<Real> values,
    Count window) noexcept;
```

Computes rolling sample standard deviation.

### `autocorrelation`

``` cpp
Result<Real> autocorrelation(
    ConstSpan<Real> values,
    Count lag = 1) noexcept;
```

Computes the sample autocorrelation at the requested lag.

------------------------------------------------------------------------

# 6. Probability

Namespace: `qf::probability`

Headers:

``` cpp
#include <qf/probability.hpp>
```

## 6.1 Normal distribution

``` cpp
struct Normal {
    Real mean = 0.0;
    Real standard_deviation = 1.0;
};
```

Functions:

``` cpp
Result<Real> normal_pdf(Real x, Normal distribution = {}) noexcept;
Result<Real> normal_cdf(Real x, Normal distribution = {}) noexcept;
Result<Real> normal_quantile(Real p, Normal distribution = {}) noexcept;
```

The standard normal is the default.

------------------------------------------------------------------------

## 6.2 Lognormal distribution

``` cpp
struct Lognormal {
    Real log_mean = 0.0;
    Real log_standard_deviation = 1.0;
};
```

Functions:

``` cpp
Result<Real> lognormal_pdf(Real x, Lognormal distribution = {}) noexcept;
Result<Real> lognormal_cdf(Real x, Lognormal distribution = {}) noexcept;
Result<Real> lognormal_quantile(Real p, Lognormal distribution = {}) noexcept;
```

The parameters describe the underlying normal variable in log space.

------------------------------------------------------------------------

## 6.3 Student-t distribution

``` cpp
struct StudentT {
    Real degrees_of_freedom = 1.0;
};
```

Functions:

``` cpp
Result<Real> student_t_pdf(Real x, StudentT distribution) noexcept;
Result<Real> student_t_cdf(Real x, StudentT distribution) noexcept;
Result<Real> student_t_quantile(Real p, StudentT distribution) noexcept;
```

------------------------------------------------------------------------

## 6.4 Regularized beta

``` cpp
Result<Real> regularized_beta(Real a, Real b, Real x) noexcept;
```

This is exposed because it is also a numerical building block for the
Student-t distribution implementation.

------------------------------------------------------------------------

## 6.5 Random numbers

Header: `qf/probability/random.hpp`

### `Xoshiro256StarStar`

A deterministic non-cryptographic pseudorandom generator.

``` cpp
explicit Xoshiro256StarStar(
    std::uint64_t seed = 0x9e3779b97f4a7c15ULL) noexcept;
```

Methods:

``` cpp
std::uint64_t next_u64() noexcept;
Real uniform_open() noexcept;
Real normal() noexcept;
Real normal(Real mean, Real standard_deviation) noexcept;
void seed_state(std::uint64_t seed) noexcept;
```

`uniform_open()` generates values strictly inside `(0,1)`, which avoids
passing zero into the logarithm in Box-Muller sampling.

`normal()` uses Box-Muller and caches the second generated normal for
the next call.

This RNG is intended for simulation and reproducible numerical work,
**not cryptography**.

------------------------------------------------------------------------

# 7. Stochastic processes

Namespace: `qf::stochastic`

Headers:

``` cpp
#include <qf/stochastic.hpp>
```

## 7.1 Brownian motion

``` cpp
struct BrownianMotion {
    static Result<Real> increment(Real dt, Real standard_normal) noexcept;
    static Real increment_unchecked(Real dt, Real standard_normal) noexcept;
};
```

The increment is

``` text
ΔW = sqrt(dt) * Z
```

for standard normal `Z`.

------------------------------------------------------------------------

## 7.2 Geometric Brownian motion

``` cpp
struct GBMParameters {
    Real mu{};
    Real sigma{};
};
```

Functions:

``` cpp
Result<Real> gbm_step(
    Real spot,
    const GBMParameters& parameters,
    Real dt,
    Real standard_normal) noexcept;

Real gbm_step_unchecked(... ) noexcept;
```

The exact lognormal step is based on

``` text
S(t+dt) = S(t) * exp((mu - sigma²/2)dt + sigma*sqrt(dt)*Z)
```

### Path simulation

``` cpp
template <typename Rng>
Status simulate_gbm_into_unchecked(
    Real initial,
    const GBMParameters& parameters,
    Real dt,
    Rng& rng,
    Span<Real> output) noexcept;
```

The output buffer is caller-owned and `output[0]` receives the initial
value.

------------------------------------------------------------------------

## 7.3 Ornstein-Uhlenbeck process

``` cpp
Result<Real> ou_step(
    Real value,
    Real theta,
    Real mu,
    Real sigma,
    Real dt,
    Real standard_normal) noexcept;

Real ou_step_unchecked(... ) noexcept;
```

The exact Gaussian transition uses

``` text
X(t+dt) = mu + (X(t)-mu)e^(-theta dt) + innovation
```

with the exact innovation variance.

Path simulation:

``` cpp
template <typename Rng>
Status simulate_ou_into_unchecked(
    Real initial,
    Real theta,
    Real mu,
    Real sigma,
    Real dt,
    Rng& rng,
    Span<Real> output) noexcept;
```

------------------------------------------------------------------------

## 7.4 Vasicek

``` cpp
struct VasicekParameters {
    Real kappa{};
    Real theta{};
    Real sigma{};
};

Result<Real> vasicek_step(... ) noexcept;
Real vasicek_step_unchecked(... ) noexcept;
```

The exact Gaussian Vasicek transition is used.

------------------------------------------------------------------------

## 7.5 CIR

``` cpp
struct CIRParameters {
    Real kappa{};
    Real theta{};
    Real sigma{};
};

Result<Real> cir_step(... ) noexcept;
Real cir_step_unchecked(... ) noexcept;
```

The current implementation uses **full-truncation Euler**, not an exact
CIR transition. The returned state is kept non-negative by truncating
the state used in the square-root diffusion term and the resulting
value.

------------------------------------------------------------------------

# 8. OU parameter estimation

Header: `qf/stochastic/ou.hpp`

``` cpp
struct OUParameters {
    Real theta{};
    Real mu{};
    Real sigma{};
    Real phi{};
    Real half_life{};
    Real dt{};
};
```

Functions:

``` cpp
Result<OUParameters> estimate_ou(
    ConstSpan<Real> values,
    Duration dt) noexcept;

Result<OUParameters> estimate_ou(
    const timeseries::TimeSeries& series) noexcept;
```

The estimator uses the exact discrete OU transition, which can be
represented as an AR(1):

``` text
X[t+1] = a + phi * X[t] + epsilon[t]
```

with

``` text
phi = exp(-theta * dt)
mu  = intercept / (1 - phi)
theta = -log(phi) / dt
```

and the innovation variance is mapped to `sigma`.

The `TimeSeries` overload requires strictly increasing, equally spaced
timestamps.

`half_life` is the mean-reversion half-life:

``` text
ln(2) / theta
```

------------------------------------------------------------------------

# 9. Optimization

Namespace: `qf::optimization`

Headers:

``` cpp
#include <qf/optimization.hpp>
```

## 9.1 Root finding

Header: `qf/optimization/root_finding.hpp`

### `RootResult`

Contains the root, function value, iteration count, and convergence
state used by the solver.

### `RootOptions`

Controls maximum iterations and absolute/relative tolerances used by
root finding.

Functions:

``` cpp
bisection(f, lo, hi, options = {})
brent(f, lo, hi, options = {})
```

and a Newton solver with a callable derivative are provided by the
header's templated API.

The bracketed solvers require a valid bracket with opposite function
signs.

### Bisection

Robust bracketed root finding. It is slower than interpolation-based
methods but has predictable convergence when the bracket is valid.

### Brent

Combines bracketing with interpolation and bisection-style safeguards.

### Newton

Uses a function and derivative to obtain fast local convergence. It is
not globally bracketed in the same way as bisection/Brent, so callers
should use appropriate initial conditions.

------------------------------------------------------------------------

## 9.2 L-BFGS

Header: `qf/optimization/lbfgs.hpp`

### `LbfgsOptions`

Controls:

``` text
memory/history size
maximum iterations
gradient tolerance
step tolerance
Armijo parameter
backtracking factor
minimum step
```

### `LbfgsWorkspace`

Caller-owned memory for the optimizer. This is the key
allocation-control mechanism.

``` cpp
bool valid(Count n, Count m) const noexcept;
```

The workspace stores the history and temporary vectors needed by the
two-loop recursion.

### `OptimizeResult`

Reports the final objective value, gradient norm, iteration count, and
convergence status.

### L-BFGS entry point

The optimizer is templated on the objective/gradient callable. The
objective supplies a function value and gradient for a point.

The algorithm uses the standard L-BFGS two-loop recursion with Armijo
backtracking.

A typical usage pattern is:

``` cpp
qf::optimization::LbfgsWorkspace workspace(...);

// Allocate workspace once.
// Call the optimizer repeatedly without per-iteration allocations.
```

The exact callable signature should be taken from `lbfgs.hpp`; the API
is intentionally template-based so the objective can be inlined.

------------------------------------------------------------------------

# 10. Financial types

Namespace: `qf::finance`

Header:

``` cpp
#include <qf/finance.hpp>
```

This is where QFLib adds financial semantics to the generic `Real` type.

## 10.1 Available scalar types

``` cpp
Rate
InterestRate
Yield
Spread
Volatility
Variance
Probability
DiscountFactor
Price
PriceDelta
Quantity
Notional
CashAmount
Weight
ZScore
Beta
Correlation
Return
LogReturn
YearFraction
Frequency
Percent
BasisPoints
```

Each is a strong scalar backed by one `Real`.

Example:

``` cpp
using namespace qf::finance;

Price spot{100.0};
Volatility sigma{0.20};
InterestRate rate{0.05};
Quantity quantity{1000.0};
```

A `Price` cannot accidentally be added to a `Volatility` because they
are different C++ types.

------------------------------------------------------------------------

## 10.2 Financial algebra

Some operations have domain-specific return types.

``` text
Price - Price                  -> PriceDelta
Price + PriceDelta             -> Price
Price - PriceDelta             -> Price
Price * Quantity               -> Notional
DiscountFactor * Notional      -> CashAmount
Volatility * Volatility        -> Variance
InterestRate * YearFraction    -> Real
Rate * YearFraction            -> Real
Weight * Weight                -> Real
```

For example:

``` cpp
Price p0{100.0};
Price p1{103.0};
PriceDelta move = p1 - p0;
Price p2 = p0 + move;

Quantity q{50.0};
Notional n = p1 * q;
```

`Price` and `Return` are deliberately non-additive as generic strong
scalars. Domain-specific operations are exposed explicitly where
appropriate.

------------------------------------------------------------------------

## 10.3 Validity checks

Generic strong scalars require a finite underlying value. Some types add
domain constraints.

``` cpp
valid(Rate)
valid(InterestRate)
valid(Yield)
valid(Spread)
valid(Volatility)
valid(Variance)
valid(Probability)
valid(DiscountFactor)
valid(Price)
valid(PriceDelta)
valid(Quantity)
valid(Notional)
valid(CashAmount)
valid(Weight)
valid(ZScore)
valid(Beta)
valid(Correlation)
valid(Return)
valid(LogReturn)
valid(YearFraction)
valid(Frequency)
valid(Percent)
valid(BasisPoints)
```

Important domain rules currently include:

``` text
Probability       0 <= p <= 1
DiscountFactor    x > 0
Volatility        x >= 0
Variance          x >= 0
Frequency         x > 0
YearFraction      x >= 0
Correlation       -1 <= x <= 1
Return            x > -1
Weight            finite
```

Construction itself is intentionally unchecked. Validate values at
data/API boundaries when required.

------------------------------------------------------------------------

## 10.4 Percentage and basis-point conversion

``` cpp
InterestRate from_percent(Percent percent) noexcept;
InterestRate from_basis_points(BasisPoints basis_points) noexcept;
Spread spread_from_percent(Percent percent) noexcept;
Spread spread_from_basis_points(BasisPoints basis_points) noexcept;
Percent to_percent(InterestRate rate) noexcept;
BasisPoints to_basis_points(InterestRate rate) noexcept;
BasisPoints to_basis_points(Spread spread) noexcept;
```

Conversions use:

``` text
1 percent = 0.01
1 basis point = 0.0001
```

Example:

``` cpp
InterestRate r = from_percent(Percent{5.0});
// r == 0.05

BasisPoints bp = to_basis_points(r);
// bp == 500
```

------------------------------------------------------------------------

# 11. Financial analytics

Header: `qf/finance/analytics.hpp`

These APIs combine financial types with generic numerical/statistical
kernels.

## 11.1 Simple returns

``` cpp
Status simple_returns_into(
    ConstSpan<Price> prices,
    Span<Return> output,
    Count lag = 1) noexcept;
```

The caller owns `output`.

Unchecked form:

``` cpp
void simple_returns_into_unchecked(
    ConstSpan<Price> prices,
    Span<Return> output,
    Count lag = 1) noexcept;
```

Allocating convenience form:

``` cpp
Result<linalg::Vector<Return>> simple_returns(
    ConstSpan<Price> prices,
    Count lag = 1) noexcept;
```

The return is

``` text
P[t] / P[t-lag] - 1
```

------------------------------------------------------------------------

## 11.2 Log returns

``` cpp
Status log_returns_into(
    ConstSpan<Price> prices,
    Span<LogReturn> output,
    Count lag = 1) noexcept;

void log_returns_into_unchecked(... ) noexcept;

Result<linalg::Vector<LogReturn>> log_returns(
    ConstSpan<Price> prices,
    Count lag = 1) noexcept;
```

The logarithmic return is

``` text
log(P[t] / P[t-lag])
```

------------------------------------------------------------------------

## 11.3 Volatility

``` cpp
Result<Volatility> volatility(ConstSpan<Return> returns) noexcept;
```

Returns the sample standard deviation of the return series.

Annualization is intentionally not implicit; if a strategy needs
annualized volatility, the scaling convention should be explicit at the
caller/model layer.

------------------------------------------------------------------------

## 11.4 Z-scores

``` cpp
Status zscore_into(
    ConstSpan<Return> returns,
    Span<ZScore> output) noexcept;

Result<linalg::Vector<ZScore>> zscores(
    ConstSpan<Return> returns) noexcept;
```

Each observation is standardized using the sample mean and sample
standard deviation.

------------------------------------------------------------------------

## 11.5 Covariance and correlation

``` cpp
Result<Variance> covariance(
    ConstSpan<Return> x,
    ConstSpan<Return> y) noexcept;

Result<Correlation> correlation(
    ConstSpan<Return> x,
    ConstSpan<Return> y) noexcept;
```

These are finance-typed wrappers around the generic statistical
operations.

------------------------------------------------------------------------

# 12. Financial streaming

Header: `qf/finance/streaming.hpp`

These classes are thin financial facades over `qf::stats` online
algorithms.

## 12.1 `OnlineReturnStats`

``` cpp
Status update(Return x) noexcept;
void update_unchecked(Return x) noexcept;
void reset() noexcept;
Count count() const noexcept;
Real mean_value() const noexcept;
Return mean() const noexcept;
Variance variance() const noexcept;
Volatility volatility() const noexcept;
```

Use this when returns arrive one observation at a time and retaining the
full history is unnecessary.

------------------------------------------------------------------------

## 12.2 `OnlineReturnCovariance`

``` cpp
Status update(Return x, Return y) noexcept;
void update_unchecked(Return x, Return y) noexcept;
void reset() noexcept;
Count count() const noexcept;
Variance covariance() const noexcept;
Correlation correlation() const noexcept;
Volatility volatility_x() const noexcept;
Volatility volatility_y() const noexcept;
```

It is backed by `stats::OnlineCovariance`.

This is useful for streaming pairwise risk/factor calculations.

------------------------------------------------------------------------

# 13. Portfolio

Namespace: `qf::portfolio`

Header:

``` cpp
#include <qf/portfolio.hpp>
```

## 13.1 Weighted return

``` cpp
Status weighted_return_into(
    ConstSpan<finance::Weight> weights,
    ConstSpan<finance::Return> returns,
    finance::Return& output) noexcept;

finance::Return weighted_return_unchecked(
    ConstSpan<finance::Weight> weights,
    ConstSpan<finance::Return> returns) noexcept;

Result<finance::Return> weighted_return(
    ConstSpan<finance::Weight> weights,
    ConstSpan<finance::Return> returns) noexcept;
```

Computes

``` text
R_p = sum_i w_i R_i
```

The library does not automatically normalize weights.

------------------------------------------------------------------------

## 13.2 Portfolio variance

``` cpp
Status portfolio_variance_into(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> covariance,
    Count dimension,
    finance::Variance& output) noexcept;

finance::Variance portfolio_variance_unchecked(... ) noexcept;
Result<finance::Variance> portfolio_variance(... ) noexcept;
```

The covariance matrix is represented as a flat row-major array of length
`dimension * dimension`.

The calculation is

``` text
sigma_p² = w' C w
```

The implementation exploits covariance symmetry to avoid redundant
off-diagonal work.

------------------------------------------------------------------------

## 13.3 Portfolio volatility

``` cpp
Status portfolio_volatility_into(... ) noexcept;
Result<finance::Volatility> portfolio_volatility(... ) noexcept;
```

Computes

``` text
sqrt(w' C w)
```

------------------------------------------------------------------------

## 13.4 Factor exposure

``` cpp
Real factor_exposure_unchecked(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> loadings) noexcept;

Result<Real> factor_exposure(
    ConstSpan<finance::Weight> weights,
    ConstSpan<Real> loadings) noexcept;
```

Computes the weighted factor exposure

``` text
sum_i w_i * loading_i
```

------------------------------------------------------------------------

# 14. Risk

Namespace: `qf::risk`

Header:

``` cpp
#include <qf/risk.hpp>
```

## 14.1 Sharpe ratio

``` cpp
Result<Real> sharpe_ratio(
    finance::Return mean_return,
    finance::Volatility volatility,
    finance::InterestRate risk_free = finance::InterestRate{0.0}) noexcept;
```

Computes

``` text
(mean_return - risk_free) / volatility
```

The inputs must use compatible time horizons. QFLib does not silently
annualize the inputs.

------------------------------------------------------------------------

## 14.2 Tracking error

``` cpp
Result<finance::Volatility> tracking_error(
    ConstSpan<finance::Return> portfolio_returns,
    ConstSpan<finance::Return> benchmark_returns) noexcept;
```

Computes the standard deviation of active returns:

``` text
active_t = portfolio_t - benchmark_t
tracking_error = std(active)
```

------------------------------------------------------------------------

## 14.3 Information ratio

``` cpp
Result<Real> information_ratio(
    ConstSpan<finance::Return> portfolio_returns,
    ConstSpan<finance::Return> benchmark_returns) noexcept;
```

Computes active mean divided by tracking error.

------------------------------------------------------------------------

## 14.4 Maximum drawdown

``` cpp
Result<finance::Return> maximum_drawdown(
    ConstSpan<finance::Price> prices) noexcept;
```

Tracks the running peak and computes the largest peak-to-trough
percentage decline.

------------------------------------------------------------------------

## 14.5 Historical VaR

Caller-owned scratch form:

``` cpp
Status historical_var_into(
    ConstSpan<finance::Return> returns,
    finance::Probability confidence,
    Span<finance::Return> scratch,
    finance::Return& output) noexcept;
```

Unchecked form:

``` cpp
finance::Return historical_var_unchecked(
    ConstSpan<finance::Return> returns,
    finance::Probability confidence,
    Span<finance::Return> scratch) noexcept;
```

Convenience form:

``` cpp
Result<finance::Return> historical_var(
    ConstSpan<finance::Return> returns,
    finance::Probability confidence,
    Span<finance::Return> scratch) noexcept;
```

The implementation uses an order-statistics selection rather than a full
sort where possible.

The exact interpretation of the returned sign should be kept consistent
with the return-series convention used by the caller.

------------------------------------------------------------------------

## 14.6 Expected shortfall

``` cpp
Status expected_shortfall_into(... ) noexcept;
finance::Return expected_shortfall_unchecked(... ) noexcept;
Result<finance::Return> expected_shortfall(... ) noexcept;
```

Expected shortfall averages losses beyond the selected historical VaR
threshold.

The caller supplies scratch storage so repeated calculations can avoid
allocating.

------------------------------------------------------------------------

# 15. Pricing

Namespace: `qf::pricing`

Header:

``` cpp
#include <qf/pricing.hpp>
```

## 15.1 Option type

``` cpp
enum class OptionType {
    call,
    put
};
```

------------------------------------------------------------------------

## 15.2 Black-Scholes inputs

``` cpp
struct BlackScholesParameters {
    finance::Price spot;
    finance::Price strike;
    finance::InterestRate rate;
    finance::Yield dividend_yield;
    finance::Volatility volatility;
    finance::YearFraction time_to_expiry;
};
```

------------------------------------------------------------------------

## 15.3 Black-76 inputs

``` cpp
struct Black76Parameters {
    finance::Price forward;
    finance::Price strike;
    finance::InterestRate rate;
    finance::Volatility volatility;
    finance::YearFraction time_to_expiry;
};
```

------------------------------------------------------------------------

## 15.4 Greeks

``` cpp
struct Greeks {
    Real delta;
    Real gamma;
    Real vega;
    Real theta;
    Real rho;
};
```

------------------------------------------------------------------------

## 15.5 Black-Scholes price

``` cpp
Result<finance::Price> black_scholes_price(
    const BlackScholesParameters& parameters,
    OptionType type) noexcept;

Real black_scholes_price_unchecked(
    const BlackScholesParameters& parameters,
    OptionType type) noexcept;
```

The standard continuous-dividend Black-Scholes model is used.

Conceptually for a call:

``` text
C = S exp(-qT) N(d1) - K exp(-rT) N(d2)
```

where

``` text
d1 = [ln(S/K) + (r-q+sigma²/2)T] / (sigma sqrt(T))
d2 = d1 - sigma sqrt(T)
```

The put is obtained from the corresponding put formula.

------------------------------------------------------------------------

## 15.6 Black-Scholes Greeks

``` cpp
Result<Greeks> black_scholes_greeks(
    const BlackScholesParameters& parameters,
    OptionType type) noexcept;

Greeks black_scholes_greeks_unchecked(... ) noexcept;
```

The implementation provides delta, gamma, vega, theta, and rho.

------------------------------------------------------------------------

## 15.7 Black-76

``` cpp
Result<finance::Price> black76_price(
    const Black76Parameters& parameters,
    OptionType type) noexcept;

Real black76_price_unchecked(... ) noexcept;
```

Black-76 prices options from a forward price rather than a spot price.

------------------------------------------------------------------------

## 15.8 Implied volatility

``` cpp
struct ImpliedVolatilityOptions {
    Count max_iterations = 100;
    Real absolute_tolerance = 1e-10;
    Real relative_tolerance = 1e-9;
    Real initial_upper_bound = 1.0;
    Real maximum_upper_bound = 16.0;
};
```

Function:

``` cpp
Result<finance::Volatility> black_scholes_implied_volatility(
    finance::Price market_price,
    const BlackScholesParameters& base_parameters,
    OptionType type,
    ImpliedVolatilityOptions options = {}) noexcept;
```

The current implementation uses safeguarded Newton iterations with
analytic vega and a maintained volatility bracket.

The input `base_parameters.volatility` is not treated as the answer; it
is simply part of the parameter object passed to the function.

------------------------------------------------------------------------

## 15.9 Binomial pricing

Caller-owned workspace form:

``` cpp
Status binomial_price_into_unchecked(
    finance::Price spot,
    finance::Price strike,
    finance::InterestRate rate,
    finance::Yield dividend_yield,
    finance::Volatility volatility,
    finance::YearFraction time_to_expiry,
    Count steps,
    OptionType type,
    bool american,
    Span<Real> workspace,
    finance::Price& output) noexcept;
```

Convenience form:

``` cpp
Result<finance::Price> binomial_price(
    finance::Price spot,
    finance::Price strike,
    finance::InterestRate rate,
    finance::Yield dividend_yield,
    finance::Volatility volatility,
    finance::YearFraction time_to_expiry,
    Count steps,
    OptionType type,
    bool american = false) noexcept;
```

The implementation uses O(steps²) time and O(steps) working memory. It
supports European and American exercise.

------------------------------------------------------------------------

# 16. Market data and reference-data views

Namespace: `qf::market`

Header:

``` cpp
#include <qf/market.hpp>
```

This module is deliberately small. It provides compact data types and
non-owning reference-data views rather than network/feed infrastructure.

------------------------------------------------------------------------

## 16.1 IDs

``` cpp
using InstrumentId = std::uint32_t;
using VenueId = std::uint16_t;

inline constexpr InstrumentId invalid_instrument = 0;
inline constexpr VenueId invalid_venue = 0;
```

------------------------------------------------------------------------

## 16.2 `Quote`

``` cpp
struct Quote {
    Timestamp timestamp;
    InstrumentId instrument;
    VenueId venue;
    finance::Price bid;
    finance::Price ask;
    finance::Quantity bid_size;
    finance::Quantity ask_size;
};
```

Methods:

``` cpp
bool valid() const noexcept;
finance::PriceDelta spread() const noexcept;
```

The validity checks include IDs, timestamp, finite/non-negative prices
and sizes, and a non-crossed top of book.

------------------------------------------------------------------------

## 16.3 `Trade`

``` cpp
struct Trade {
    Timestamp timestamp;
    InstrumentId instrument;
    VenueId venue;
    finance::Price price;
    finance::Quantity quantity;
    std::uint8_t aggressor_side;
};
```

`aggressor_side` uses:

``` text
0 = unknown
1 = buy
2 = sell
```

Methods:

``` cpp
bool valid() const noexcept;
```

------------------------------------------------------------------------

## 16.4 `Bar`

``` cpp
struct Bar {
    Timestamp timestamp;
    InstrumentId instrument;
    VenueId venue;
    finance::Price open;
    finance::Price high;
    finance::Price low;
    finance::Price close;
    finance::Quantity volume;
    std::uint32_t trade_count;
};
```

`valid()` checks the basic OHLC ordering and non-negative volume
constraints.

------------------------------------------------------------------------

## 16.5 `Level`

``` cpp
struct Level {
    finance::Price price;
    finance::Quantity quantity;

    bool valid() const noexcept;
};
```

------------------------------------------------------------------------

## 16.6 `OrderBookSnapshot<Depth>`

A fixed-depth, fixed-capacity order-book snapshot.

``` cpp
template <std::size_t Depth>
struct OrderBookSnapshot;
```

Storage is two fixed `std::array<Level, Depth>` instances.

Fields:

``` cpp
Timestamp timestamp;
InstrumentId instrument;
VenueId venue;
std::array<Level, Depth> bids;
std::array<Level, Depth> asks;
std::uint16_t bid_count;
std::uint16_t ask_count;
```

Methods:

``` cpp
Span<const Level> bid_levels() const noexcept;
Span<const Level> ask_levels() const noexcept;
const Level* best_bid() const noexcept;
const Level* best_ask() const noexcept;
bool valid() const noexcept;
```

The validity rules require:

-   valid IDs and timestamp
-   counts no greater than `Depth`
-   bid prices strictly descending
-   ask prices strictly ascending
-   no crossed top of book

The structure is fixed-size and allocation-free.

------------------------------------------------------------------------

## 16.7 Quote analytics

Header: `qf/market/analytics.hpp`

### Mid price

``` cpp
finance::Price mid_price_unchecked(const Quote& q) noexcept;
```

``` text
(bid + ask) / 2
```

### Microprice

``` cpp
finance::Price microprice_unchecked(const Quote& q) noexcept;
```

Uses bid/ask sizes to weight the opposite side of the book.

### Book imbalance

``` cpp
finance::Weight book_imbalance_unchecked(const Quote& q) noexcept;
```

Computes

``` text
(bid_size - ask_size) / (bid_size + ask_size)
```

with zero returned when the total displayed size is zero.

### Spread

``` cpp
finance::PriceDelta spread_unchecked(const Quote& q) noexcept;
```

Returns `ask - bid`.

### VWAP

``` cpp
finance::Price vwap_unchecked(ConstSpan<Trade> trades) noexcept;
```

Computes

``` text
sum(price_i * quantity_i) / sum(quantity_i)
```

and returns zero when total quantity is zero.

These functions are explicitly unchecked because they are intended to be
tiny market-data kernels.

------------------------------------------------------------------------

## 16.8 Discount-factor curve view

Header: `qf/market/curve.hpp`

``` cpp
struct CurveView {
    ConstSpan<finance::YearFraction> maturities;
    ConstSpan<finance::DiscountFactor> discount_factors;

    bool valid() const noexcept;
    std::size_t size() const noexcept;
};
```

`valid()` requires matching non-empty arrays, valid financial values,
and strictly increasing maturities.

Interpolation:

``` cpp
finance::DiscountFactor interpolate_discount_factor(
    CurveView curve,
    finance::YearFraction maturity) noexcept;

finance::DiscountFactor interpolate_discount_factor_unchecked(
    CurveView curve,
    finance::YearFraction maturity,
    std::size_t& interval_hint) noexcept;
```

The unchecked form accepts an interval hint that can be reused across
sequential queries.

------------------------------------------------------------------------

## 16.9 Volatility surface view

Header: `qf/market/surface.hpp`

``` cpp
struct VolatilitySurfaceView {
    ConstSpan<finance::YearFraction> expiries;
    ConstSpan<finance::Price> strikes;
    ConstSpan<finance::Volatility> volatilities;

    bool valid() const noexcept;
    std::size_t index(std::size_t expiry, std::size_t strike) const noexcept;
    finance::Volatility at_unchecked(
        std::size_t expiry,
        std::size_t strike) const noexcept;
};
```

The volatility data is row-major:

``` text
index(expiry, strike) = expiry * number_of_strikes + strike
```

`valid()` requires sorted expiry/strike axes and exactly
`expiries.size() * strikes.size()` volatility values.

This is currently a view only; it does not implement surface
interpolation or calibration.

------------------------------------------------------------------------

# 17. Header-only vs compiled code

QFLib intentionally mixes header-only and compiled components.

### Mostly header-only

``` text
core
strong scalar types
small math functions
container templates
views
optimization templates
random generator
small market-data kernels
```

### Compiled implementations

``` text
math/scalar.cpp
math/reductions.cpp
math/polynomial.cpp
linalg/algorithms.cpp
stats/*.cpp
timeseries/*.cpp
stochastic/*.cpp
finance/analytics.cpp
finance/portfolio.cpp
finance/risk.cpp
pricing/black_scholes.cpp
probability/distributions.cpp
market/curve.cpp
```

The split is intentional: templates and tiny operations benefit from
being visible to the compiler, while larger algorithms avoid forcing
their implementation into every translation unit.

------------------------------------------------------------------------

# 18. Checked vs unchecked APIs

One recurring QFLib pattern is:

``` text
checked API
    |
    v
validate once
    |
    v
unchecked hot-path API
```

For example:

``` cpp
finance::simple_returns_into(prices, output);
```

performs input/output validation and returns a `Status`.

Once the caller knows its data satisfies the preconditions:

``` cpp
finance::simple_returns_into_unchecked(prices, output);
```

can be used to remove repeated validation from the inner loop.

This pattern appears in:

-   financial return calculations
-   online statistics
-   stochastic process steps
-   linear algebra kernels
-   portfolio/risk calculations
-   binomial pricing workspace operations
-   market analytics

Unchecked functions should only be used when their documented
preconditions are already satisfied.

------------------------------------------------------------------------

# 19. Memory and performance model

QFLib favors:

-   contiguous arrays
-   row-major matrices
-   non-owning spans
-   caller-owned scratch buffers
-   fixed-size market-data structures
-   one-pass online statistics
-   O(1) rolling updates where possible
-   no allocation in hot numerical loops
-   simple data structures that the compiler can optimize aggressively

For example, a streaming return statistic does not need to retain all
historical returns:

``` text
new return
    |
    v
OnlineReturnStats
    |
    +--> count
    +--> mean
    +--> M2
```

Similarly, portfolio variance uses the covariance matrix directly rather
than constructing unnecessary intermediate objects.

------------------------------------------------------------------------

# 20. Numerical conventions

Several conventions are worth knowing before using the library.

### Sample statistics

Variance and standard deviation use the sample denominator `n-1` when at
least two observations are available.

### Prices

Financial return APIs expect positive, finite prices on their checked
paths.

### Returns

A simple return must satisfy `return > -1` under the current financial
validity rule.

### Probabilities

Probabilities are represented as `[0, 1]` values.

### Volatility

Volatility is non-negative and is not automatically annualized.

### Time

Core timestamps use integer nanoseconds. Financial year fractions are
separate strong scalar types.

### Tolerances

Floating-point equality should normally use `almost_equal` or a
problem-specific tolerance rather than exact equality.

------------------------------------------------------------------------

# 21. Typical usage

## A small numerical example

``` cpp
#include <qf/qf.hpp>
#include <iostream>

int main() {
    qf::linalg::Vector<qf::Real> x{1.0, 2.0, 3.0};

    auto result = qf::linalg::norm(x.span());
    if (!result) {
        std::cerr << result.error().message() << '\n';
        return 1;
    }

    std::cout << result.value() << '\n';
}
```

## Financial returns

``` cpp
#include <qf/finance.hpp>
#include <qf/linalg/vector.hpp>

using namespace qf;
using namespace qf::finance;

int main() {
    linalg::Vector<Price> prices{
        Price{100.0},
        Price{101.0},
        Price{99.0},
        Price{102.0}
    };

    auto returns = simple_returns(prices.span());
    if (!returns) {
        return 1;
    }

    auto vol = volatility(returns.value().span());
    if (!vol) {
        return 1;
    }

    // vol.value() is a finance::Volatility.
}
```

## Streaming statistics

``` cpp
qf::finance::OnlineReturnStats stats;

stats.update(qf::finance::Return{0.01});
stats.update(qf::finance::Return{-0.005});
stats.update(qf::finance::Return{0.008});

qf::finance::Volatility sigma = stats.volatility();
```

For a validated data feed, the hot path can use:

``` cpp
stats.update_unchecked(return_value);
```

after the relevant preconditions have already been enforced.

## Black-Scholes

``` cpp
qf::pricing::BlackScholesParameters p{
    qf::finance::Price{100.0},
    qf::finance::Price{100.0},
    qf::finance::InterestRate{0.05},
    qf::finance::Yield{0.0},
    qf::finance::Volatility{0.20},
    qf::finance::YearFraction{1.0}
};

auto price = qf::pricing::black_scholes_price(
    p,
    qf::pricing::OptionType::call);
```

------------------------------------------------------------------------


# Project structure

``` text
QFLib/
├── include/qf/
│   ├── core/
│   ├── math/
│   ├── linalg/
│   ├── stats/
│   ├── timeseries/
│   ├── probability/
│   ├── stochastic/
│   ├── optimization/
│   ├── finance/
│   ├── portfolio/
│   ├── risk/
│   ├── pricing/
│   └── market/
│
├── src/qf/
│   └── compiled implementations
│
├── tests/
│   └── module-level tests
│
├── benchmarks/
│   └── microbenchmarks
│
├── examples/
│   └── small usage examples
│
├── docs/
│   └── deeper design notes
│
└── cmake/
    └── build configuration helpers
```

The intended dependency direction is from higher-level applications
toward QFLib, never the reverse.

------------------------------------------------------------------------

# License

See [`LICENSE`](LICENSE).
