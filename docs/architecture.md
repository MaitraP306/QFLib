# QFLib Phase 0 Architecture

Phase 0 establishes the build and repository foundation only.

## Dependency direction

Future quantitative modules will follow:

core
  -> math
  -> linalg
  -> stats / timeseries / probability / stochastic
  -> optimization / signal / portfolio / risk / pricing
  -> downstream quant engine

QFLib must never depend on strategy implementations or the quant pipeline.

## Public target

Downstream projects consume:

    qf::qflib

Public headers live under:

    include/qf/

## Performance policy

The library will prefer:
- value semantics
- contiguous storage
- views/spans
- compile-time polymorphism for numerical kernels
- minimal allocation
- predictable hot paths

Optimization such as SIMD and native CPU tuning will be introduced only after profiling.

## Phase 0 intentionally excludes

- numerical algorithms
- trading strategies
- market-data connectors
- Kafka/WebSocket infrastructure
- backtesting
- portfolio execution
- external numerical libraries

Those belong to later phases.

## Phase 1 Core Contracts

The `qf::core` layer is the lowest-level public API. It owns stable representations
and contracts used by numerical modules:

- **Types:** `Real`, `Index`, `Count`, and `TimestampRep`.
- **Concepts:** `FloatingPoint`, `Integral`, and related arithmetic constraints.
- **Numerical policy:** `Tolerance`, finite/NaN/infinity predicates, approximate equality,
  and scalar clamping.
- **Views:** `Span`/`ConstSpan`, backed by `std::span` and therefore non-owning.
- **Time:** `Timestamp` is an integer nanosecond epoch value; `Duration` is a signed
  nanosecond interval. Neither type embeds timezone or calendar semantics.
- **Errors:** `Status` carries a stable error code and non-owning message; `Result<T>`
  represents either a value or a `Status` without exceptions in the numerical API.

Core types are deliberately small, value-oriented, and suitable for use in hot loops.
No core type depends on a strategy, market-data source, storage engine, or orchestration
layer.

## Phase 2 — Math

The `qf::math` layer provides the scalar mathematical kernel and small, reusable
reductions used by later numerical modules. It intentionally has no dependency
on linear algebra, statistics, pricing, or strategy code.

Current primitives include stable scalar transforms (`log1pexp`, `sigmoid`),
common elementary functions, interpolation, polynomial evaluation via Horner's
method, and compensated reductions such as `sum` and `sum_squares`.

## Phase 3: Linear algebra storage

The linear algebra storage layer provides owning contiguous vectors and row-major matrices plus non-owning views. Structural operations live here; decompositions and advanced matrix algorithms are deferred to Phase 4.

## Performance Contract

QFLib is designed as a reusable numerical kernel for latency-sensitive quantitative workloads.
The performance hierarchy is:

1. algorithm and asymptotic complexity
2. contiguous/cache-friendly data layout
3. elimination of allocations and copies from hot paths
4. compiler optimization and link-time optimization
5. SIMD/vectorization
6. multithreading only where the workload benefits
7. hand-tuned kernels only after profiling

Numerical correctness takes precedence over unsafe floating-point transformations. In particular,
QFLib does not enable `-ffast-math` globally. Release builds use aggressive ordinary optimization,
fast floating-point contraction where supported, and optional LTO. Host-specific `-march=native`
is opt-in rather than part of the portable library ABI.

### API tiers

The library provides checked convenience APIs for research and application code and allocation-free
or caller-owned-output APIs for latency-sensitive loops. Hot-path stateful components should be small,
trivially movable, contiguous, and free of virtual dispatch, `std::function`, `shared_ptr`, and hidden
heap allocation.

### Translation units

Small compile-time abstractions and owning templates remain header-only. Substantial non-template
numerical implementations live in `.cpp` translation units. This keeps compile-time and ABI boundaries
clean while still allowing LTO to recover cross-translation-unit optimization in production builds.


## Phase 6 — PCA

PCA is implemented as a generic statistical primitive, not as a strategy component. `qf::stats::pca_fit` computes feature means, forms the sample covariance matrix, and delegates the symmetric eigendecomposition to `qf::linalg::symmetric_eigen`. Components are exposed as rows in the fitted PCA model so projection is a contiguous dot-product workload.

The implementation avoids an intermediate centered observation matrix during fitting, reducing peak memory traffic. The current eigensolver is Jacobi-based and intentionally dependency-free; future profiling can justify a more specialized symmetric eigensolver for larger dimensions without changing the PCA public contract.


## Phase 7 — Time Series

`qf::timeseries` provides compact structure-of-arrays time series storage and linear-time
transformations such as differences, simple/log returns, rolling statistics, and autocorrelation.
Validated APIs are separated from trusted append paths so market-data ingestion can reserve capacity
and avoid repeated validation once upstream invariants are established.

## Phase 8 — Ornstein-Uhlenbeck estimation

`qf::stochastic::estimate_ou` estimates the stationary continuous-time OU process

    dX_t = theta (mu - X_t) dt + sigma dW_t

from equally spaced observations. The implementation uses the exact discrete transition

    X_{t+dt} = alpha + phi X_t + epsilon_t,
    phi = exp(-theta dt),
    alpha = mu (1 - phi),

and estimates the AR(1) sufficient statistics directly in one pass. It does not construct a
design matrix or temporary lagged vectors. The conversion from `phi` to `theta` and `1-phi` uses
`log`/`expm1` identities to preserve precision when persistence is close to one.

The resulting model exposes `theta`, `mu`, `sigma`, `phi`, and mean-reversion half-life. A
`TimeSeries` overload verifies exact equal timestamp spacing before estimation. This is a generic
stochastic-process primitive and contains no Avellaneda-Lee-specific logic.
