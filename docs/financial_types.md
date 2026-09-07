# Financial-domain type layer

QFLib uses zero-overhead strong types at financial API boundaries while keeping
numerical kernels based on `Real`/contiguous spans. A financial scalar contains
exactly one `Real`, is trivially copyable and standard-layout, and introduces no
virtual dispatch or dynamic allocation.

## Design rules

1. **No implicit cross-domain conversions.** `InterestRate`, `Yield`, `Spread`,
   `Price`, `Quantity`, `Volatility`, etc. are distinct C++ types.
2. **No financial objects in hot numerical containers.** Vectorized kernels use
   `Real`, `Vector<Real>` and spans; domain types are used at API boundaries.
3. **Validation is explicit and off the hot path.** Constructors are unchecked
   and `constexpr/noexcept`; `qf::finance::valid(...)` validates external inputs once.
4. **Financial algebra is explicit.** For example, `Price * Quantity -> Notional`
   and `DiscountFactor * Notional -> CashAmount`. Unrelated products do not compile.
5. **Rate units are explicit.** Percent and basis-point inputs must be converted
   through named functions such as `from_basis_points(...)`.

## Provided types

`Rate`, `InterestRate`, `Yield`, `Spread`, `Volatility`, `Variance`,
`Probability`, `DiscountFactor`, `Price`, `PriceDelta`, `Quantity`, `Notional`,
`CashAmount`, `Weight`, `ZScore`, `Beta`, `Correlation`, `Return`, `LogReturn`,
`YearFraction`, `Frequency`, `Percent`, and `BasisPoints`.

## Performance contract

For every domain scalar `T`:

```cpp
static_assert(sizeof(T) == sizeof(qf::Real));
static_assert(std::is_trivially_copyable_v<T>);
static_assert(std::is_standard_layout_v<T>);
```

The layer is therefore suitable for latency-sensitive code: passing/copying a
strong scalar is equivalent in storage cost to passing/copying a `double`.
