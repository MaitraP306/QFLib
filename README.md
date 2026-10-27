# QFLib

QFLib is a C++20 library for numerical computing and quantitative finance. It is meant to provide reusable building blocks for research and trading projects, rather than a complete trading system.

## What it includes

- **Numerical tools:** core utilities, dense vectors and matrices, and linear algebra routines such as LU, QR, and Cholesky factorization.
- **Statistics and time series:** descriptive and online statistics, covariance, correlation, OLS regression, PCA, returns, and rolling calculations.
- **Probability and simulation:** common distributions, a seeded random-number generator, and stochastic processes including Brownian motion, GBM, Ornstein–Uhlenbeck, Vasicek, and CIR.
- **Optimization:** root-finding methods and L-BFGS, along with regression utilities.
- **Quantitative finance:** strongly typed financial values, portfolio and risk metrics, market curves and surfaces, and option-pricing tools including Black–Scholes and Black-76.

## Requirements

- A C++20 compiler and standard library
- CMake 3.20 or newer
- Ninja, used by the included CMake presets

The current implementation does not require an external numerical library.

## Build and test

From the project directory, run:

```bash
cmake --preset debug
cmake --build --preset debug
ctest --test-dir build/debug --output-on-failure
```

For an optimized Release build (which also enables the benchmark target in the supplied preset):

```bash
cmake --preset release
cmake --build --preset release
ctest --test-dir build/release --output-on-failure
```

The `asan-ubsan` preset is available for AddressSanitizer and UndefinedBehaviorSanitizer builds.

## Use it in a project

Include the umbrella header for the full public API:

```cpp
#include <qf/qf.hpp>
```

Or include a narrower module header, such as `<qf/stats.hpp>` or `<qf/pricing.hpp>`, to make dependencies clearer. The CMake library target is `qf::qflib`.

Financial scalar types such as `Price`, `Quantity`, and `Volatility` help prevent accidental mixing of different concepts at compile time. Their underlying values use `qf::Real` (`double`), not fixed-point decimal arithmetic.

## Repository layout

- `include/qf/` — public headers
- `src/qf/` — implementation
- `tests/` — automated tests
- `examples/` — small usage examples
- `benchmarks/` — optional microbenchmarks

QFLib is released under the MIT License. See [`LICENSE`](LICENSE) for details.
