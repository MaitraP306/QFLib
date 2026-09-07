#pragma once

#include <qf/core/error.hpp>
#include <qf/core/span.hpp>
#include <qf/core/types.hpp>
#include <qf/finance/types.hpp>

namespace qf::pricing {

enum class OptionType { call, put };

struct BlackScholesParameters final {
    finance::Price spot{};
    finance::Price strike{};
    finance::InterestRate rate{};
    finance::Yield dividend_yield{};
    finance::Volatility volatility{};
    finance::YearFraction time_to_expiry{};
};

struct Black76Parameters final {
    finance::Price forward{};
    finance::Price strike{};
    finance::InterestRate rate{};
    finance::Volatility volatility{};
    finance::YearFraction time_to_expiry{};
};

struct Greeks final {
    Real delta{};
    Real gamma{};
    Real vega{};
    Real theta{};
    Real rho{};
};

[[nodiscard]] Result<finance::Price> black_scholes_price(
    const BlackScholesParameters& parameters, OptionType type) noexcept;

[[nodiscard]] Result<Greeks> black_scholes_greeks(
    const BlackScholesParameters& parameters, OptionType type) noexcept;

[[nodiscard]] Result<finance::Price> black76_price(
    const Black76Parameters& parameters, OptionType type) noexcept;

[[nodiscard]] Real black_scholes_price_unchecked(
    const BlackScholesParameters& parameters, OptionType type) noexcept;

[[nodiscard]] Greeks black_scholes_greeks_unchecked(
    const BlackScholesParameters& parameters, OptionType type) noexcept;

[[nodiscard]] Real black76_price_unchecked(
    const Black76Parameters& parameters, OptionType type) noexcept;


struct ImpliedVolatilityOptions final {
    Count max_iterations = 100;
    Real absolute_tolerance = 1e-10;
    Real relative_tolerance = 1e-9;
    Real initial_upper_bound = 1.0;
    Real maximum_upper_bound = 16.0;
};

[[nodiscard]] Result<finance::Volatility> black_scholes_implied_volatility(
    finance::Price market_price,
    const BlackScholesParameters& base_parameters,
    OptionType type,
    ImpliedVolatilityOptions options = {}) noexcept;

[[nodiscard]] Status binomial_price_into_unchecked(
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

[[nodiscard]] Result<finance::Price> binomial_price(
    finance::Price spot,
    finance::Price strike,
    finance::InterestRate rate,
    finance::Yield dividend_yield,
    finance::Volatility volatility,
    finance::YearFraction time_to_expiry,
    Count steps,
    OptionType type,
    bool american = false) noexcept;

} // namespace qf::pricing
