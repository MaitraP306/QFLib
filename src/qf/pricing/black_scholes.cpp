#include <qf/pricing/black_scholes.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include <qf/core/numerical.hpp>
#include <qf/math/scalar.hpp>

namespace qf::pricing {
namespace {
constexpr Real inv_sqrt_two_pi = 0.398942280401432677939946059934381868;

[[nodiscard]] constexpr Real intrinsic(Real spot, Real strike, OptionType type) noexcept {
    return type == OptionType::call ? std::max(spot - strike, 0.0) : std::max(strike - spot, 0.0);
}

[[nodiscard]] constexpr Real signed_intrinsic_forward(Real forward, Real strike, OptionType type) noexcept {
    return type == OptionType::call ? std::max(forward - strike, 0.0) : std::max(strike - forward, 0.0);
}

[[nodiscard]] Real normal_pdf(Real x) noexcept {
    return inv_sqrt_two_pi * std::exp(-0.5 * x * x);
}

[[nodiscard]] Real normal_cdf(Real x) noexcept {
    return 0.5 * std::erfc(-x * 0.707106781186547524400844362104849039);
}

[[nodiscard]] bool bs_valid(const BlackScholesParameters& p) noexcept {
    return std::isfinite(p.spot.value()) && p.spot.value() > 0.0 && std::isfinite(p.strike.value()) && p.strike.value() > 0.0 && std::isfinite(p.rate.value()) && std::isfinite(p.dividend_yield.value()) && std::isfinite(p.volatility.value()) && p.volatility.value() >= 0.0 && std::isfinite(p.time_to_expiry.value()) && p.time_to_expiry.value() >= 0.0;
}

[[nodiscard]] bool b76_valid(const Black76Parameters& p) noexcept {
    return std::isfinite(p.forward.value()) && p.forward.value() > 0.0 && std::isfinite(p.strike.value()) && p.strike.value() > 0.0 && std::isfinite(p.rate.value()) && std::isfinite(p.volatility.value()) && p.volatility.value() >= 0.0 && std::isfinite(p.time_to_expiry.value()) && p.time_to_expiry.value() >= 0.0;
}

[[nodiscard]] Real bs_price_value(const BlackScholesParameters& p, OptionType type) noexcept {
    const Real s = p.spot.value();
    const Real k = p.strike.value();
    const Real r = p.rate.value();
    const Real q = p.dividend_yield.value();
    const Real sigma = p.volatility.value();
    const Real t = p.time_to_expiry.value();
    if (t == 0.0) return intrinsic(s, k, type);

    const Real sqrt_t = std::sqrt(t);
    if (sigma == 0.0) {
        const Real forward = s * std::exp((r - q) * t);
        return std::exp(-r * t) * signed_intrinsic_forward(forward, k, type);
    }

    const Real vsqrt_t = sigma * sqrt_t;
    const Real d1 = (std::log(s / k) + (r - q + 0.5 * sigma * sigma) * t) / vsqrt_t;
    const Real d2 = d1 - vsqrt_t;
    const Real df_r = std::exp(-r * t);
    const Real df_q = std::exp(-q * t);
    if (type == OptionType::call)
        return s * df_q * normal_cdf(d1) - k * df_r * normal_cdf(d2);
    return k * df_r * normal_cdf(-d2) - s * df_q * normal_cdf(-d1);
}

[[nodiscard]] Greeks bs_greeks_value(const BlackScholesParameters& p, OptionType type) noexcept {
    const Real s = p.spot.value();
    const Real k = p.strike.value();
    const Real r = p.rate.value();
    const Real q = p.dividend_yield.value();
    const Real sigma = p.volatility.value();
    const Real t = p.time_to_expiry.value();
    if (t == 0.0) {
        const Real sign = type == OptionType::call ? 1.0 : -1.0;
        const bool itm = type == OptionType::call ? s > k : s < k;
        return Greeks{itm ? sign : 0.0, 0.0, 0.0, 0.0, 0.0};
    }
    const Real sqrt_t = std::sqrt(t);
    if (sigma == 0.0) return Greeks{};
    const Real vsqrt_t = sigma * sqrt_t;
    const Real d1 = (std::log(s / k) + (r - q + 0.5 * sigma * sigma) * t) / vsqrt_t;
    const Real d2 = d1 - vsqrt_t;
    const Real df_r = std::exp(-r * t);
    const Real df_q = std::exp(-q * t);
    const Real pdf = normal_pdf(d1);
    const Real call_delta = df_q * normal_cdf(d1);
    const Real gamma = df_q * pdf / (s * vsqrt_t);
    const Real vega = s * df_q * pdf * sqrt_t;
    const Real call_theta = -(s * df_q * pdf * sigma) / (2.0 * sqrt_t) - r * k * df_r * normal_cdf(d2) + q * s * df_q * normal_cdf(d1);
    const Real call_rho = k * t * df_r * normal_cdf(d2);
    if (type == OptionType::call)
        return Greeks{call_delta, gamma, vega, call_theta, call_rho};
    return Greeks{call_delta - df_q, gamma, vega, -(s * df_q * pdf * sigma) / (2.0 * sqrt_t) + r * k * df_r * normal_cdf(-d2) - q * s * df_q * normal_cdf(-d1), -k * t * df_r * normal_cdf(-d2)};
}

[[nodiscard]] Real b76_price_value(const Black76Parameters& p, OptionType type) noexcept {
    const Real f = p.forward.value();
    const Real k = p.strike.value();
    const Real r = p.rate.value();
    const Real sigma = p.volatility.value();
    const Real t = p.time_to_expiry.value();
    const Real df = std::exp(-r * t);
    if (t == 0.0) return signed_intrinsic_forward(f, k, type);
    if (sigma == 0.0) return df * signed_intrinsic_forward(f, k, type);
    const Real vsqrt_t = sigma * std::sqrt(t);
    const Real d1 = (std::log(f / k) + 0.5 * sigma * sigma * t) / vsqrt_t;
    const Real d2 = d1 - vsqrt_t;
    if (type == OptionType::call)
        return df * (f * normal_cdf(d1) - k * normal_cdf(d2));
    return df * (k * normal_cdf(-d2) - f * normal_cdf(-d1));
}

[[nodiscard]] Status validate_implied(const BlackScholesParameters& p, Real market, const ImpliedVolatilityOptions& o) noexcept {
    if (!bs_valid(p) || !std::isfinite(market) || market < 0.0)
        return {ErrorCode::invalid_argument, "invalid Black-Scholes implied-volatility input"};
    if (!std::isfinite(o.initial_upper_bound) || !std::isfinite(o.maximum_upper_bound) ||
        o.initial_upper_bound <= 0.0 || o.maximum_upper_bound < o.initial_upper_bound ||
        o.max_iterations == 0 || o.absolute_tolerance < 0.0 || o.relative_tolerance < 0.0)
        return {ErrorCode::invalid_argument, "invalid implied-volatility options"};
    return {};
}
} // namespace

Result<finance::Price> black_scholes_price(const BlackScholesParameters& p, OptionType type) noexcept {
    if (!bs_valid(p)) return Status{ErrorCode::invalid_argument, "invalid Black-Scholes parameters"};
    return finance::Price{black_scholes_price_unchecked(p, type)};
}

Real black_scholes_price_unchecked(const BlackScholesParameters& p, OptionType type) noexcept {
    return bs_price_value(p, type);
}

Result<Greeks> black_scholes_greeks(const BlackScholesParameters& p, OptionType type) noexcept {
    if (!bs_valid(p)) return Status{ErrorCode::invalid_argument, "invalid Black-Scholes parameters"};
    return black_scholes_greeks_unchecked(p, type);
}

Greeks black_scholes_greeks_unchecked(const BlackScholesParameters& p, OptionType type) noexcept {
    return bs_greeks_value(p, type);
}

Result<finance::Price> black76_price(const Black76Parameters& p, OptionType type) noexcept {
    if (!b76_valid(p)) return Status{ErrorCode::invalid_argument, "invalid Black-76 parameters"};
    return finance::Price{black76_price_unchecked(p, type)};
}

Real black76_price_unchecked(const Black76Parameters& p, OptionType type) noexcept {
    return b76_price_value(p, type);
}

Result<finance::Volatility> black_scholes_implied_volatility(
    finance::Price market_price, const BlackScholesParameters& base,
    OptionType type, ImpliedVolatilityOptions options) noexcept {
    const Status valid = validate_implied(base, market_price.value(), options);
    if (!valid) return valid;
    if (base.time_to_expiry.value() == 0.0)
        return Status{ErrorCode::invalid_argument, "implied volatility undefined at expiry"};

    const Real s = base.spot.value(), k = base.strike.value();
    const Real r = base.rate.value(), q = base.dividend_yield.value(), t = base.time_to_expiry.value();
    const Real lower = type == OptionType::call ? std::max(0.0, s * std::exp(-q * t) - k * std::exp(-r * t)) : std::max(0.0, k * std::exp(-r * t) - s * std::exp(-q * t));
    const Real upper = type == OptionType::call ? s * std::exp(-q * t) : k * std::exp(-r * t);
    const Real market = market_price.value();
    const Real price_tol = options.absolute_tolerance + options.relative_tolerance * std::max(1.0, market);
    if (market < lower - price_tol || market > upper + price_tol)
        return Status{ErrorCode::out_of_range, "market option price outside no-arbitrage bounds"};
    if (std::abs(market - lower) <= price_tol)
        return finance::Volatility{0.0};

    BlackScholesParameters p = base;
    auto objective = [&](Real sigma) noexcept {
        p.volatility = finance::Volatility{sigma};
        return bs_price_value(p, type) - market;
    };

    Real hi = options.initial_upper_bound;
    Real fhi = objective(hi);
    while (fhi < 0.0 && hi < options.maximum_upper_bound) {
        hi = std::min(2.0 * hi, options.maximum_upper_bound);
        fhi = objective(hi);
    }
    const Real fzero = objective(0.0);
    if (fzero > price_tol || fhi < -price_tol)
        return Status{ErrorCode::numerical_failure, "could not bracket implied volatility"};

    Real lo = 0.0;
    Real x = std::min(base.volatility.value() > 0.0 ? base.volatility.value() : 0.20, hi);
    if (!(x > lo && x < hi)) x = 0.5 * hi;
    Real flo = fzero;
    for (Count iter = 0; iter < options.max_iterations; ++iter) {
        const Real fx = objective(x);
        const Real scale = options.absolute_tolerance + options.relative_tolerance * std::max(1.0, market);
        if (std::abs(fx) <= scale) return finance::Volatility{x};
        if (fx > 0.0) { hi = x; fhi = fx; }
        else { lo = x; flo = fx; }

        const Real sqrt_t = std::sqrt(t);
        const Real vsqrt_t = x * sqrt_t;
        const Real d1 = (std::log(s / k) + (r - q + 0.5 * x * x) * t) / vsqrt_t;
        const Real vega = s * std::exp(-q * t) * normal_pdf(d1) * sqrt_t;
        Real next = x;
        if (vega > std::numeric_limits<Real>::min() && std::isfinite(vega))
            next = x - fx / vega;
        if (!(next > lo && next < hi) || !std::isfinite(next))
            next = lo + 0.5 * (hi - lo);
        if (std::abs(hi - lo) <= options.absolute_tolerance + options.relative_tolerance * std::abs(next))
            return finance::Volatility{next};
        x = next;
        (void)flo; (void)fhi;
    }
    return Status{ErrorCode::numerical_failure, "implied-volatility solver did not converge"};
}

Status binomial_price_into_unchecked(
    finance::Price spot, finance::Price strike, finance::InterestRate rate,
    finance::Yield dividend_yield, finance::Volatility volatility,
    finance::YearFraction time_to_expiry, Count steps, OptionType type,
    bool american, Span<Real> workspace, finance::Price& output) noexcept {
    if (steps == 0 || workspace.size() < steps + 1)
        return {ErrorCode::invalid_argument, "binomial workspace is too small"};
    const Real s = spot.value(), k = strike.value(), r = rate.value(), q = dividend_yield.value();
    const Real sigma = volatility.value(), t = time_to_expiry.value();
    if (!(s > 0.0) || !(k > 0.0) || sigma < 0.0 || t < 0.0 || !std::isfinite(s) || !std::isfinite(k) ||
        !std::isfinite(r) || !std::isfinite(q) || !std::isfinite(sigma) || !std::isfinite(t))
        return {ErrorCode::invalid_argument, "invalid binomial parameters"};
    if (t == 0.0) { output = finance::Price{intrinsic(s, k, type)}; return {}; }

    const Real dt = t / static_cast<Real>(steps);
    const Real sqrt_dt = std::sqrt(dt);
    const Real u = std::exp(sigma * sqrt_dt);
    const Real d = 1.0 / u;
    const Real growth = std::exp((r - q) * dt);
    const Real denom = u - d;
    if (!(denom > 0.0) || !std::isfinite(denom))
        return {ErrorCode::numerical_failure, "binomial tree is degenerate"};
    const Real p = (growth - d) / denom;
    if (!(p > 0.0 && p < 1.0))
        return {ErrorCode::invalid_argument, "binomial risk-neutral probability outside (0,1)"};
    const Real disc = std::exp(-r * dt);
    const Real down_factor = std::pow(d, static_cast<Real>(steps));
    const Real ratio = u / d;
    for (Index j = 0; j <= steps; ++j) {
        const Real node = s * down_factor * std::pow(ratio, static_cast<Real>(j));
        workspace[j] = intrinsic(node, k, type);
    }
    for (Index i = steps; i-- > 0;) {
        for (Index j = 0; j <= i; ++j) {
            const Real continuation = disc * (p * workspace[j + 1] + (1.0 - p) * workspace[j]);
            if (american) {
                const Real node = s * std::pow(u, static_cast<Real>(j)) * std::pow(d, static_cast<Real>(i - j));
                workspace[j] = std::max(continuation, intrinsic(node, k, type));
            } else {
                workspace[j] = continuation;
            }
        }
    }
    output = finance::Price{workspace[0]};
    return {};
}

Result<finance::Price> binomial_price(
    finance::Price spot, finance::Price strike, finance::InterestRate rate,
    finance::Yield dividend_yield, finance::Volatility volatility,
    finance::YearFraction time_to_expiry, Count steps, OptionType type,
    bool american) noexcept {
    if (steps == 0) return Status{ErrorCode::invalid_argument, "binomial steps must be positive"};
    std::vector<Real> workspace(steps + 1);
    finance::Price output{};
    const Status status = binomial_price_into_unchecked(spot, strike, rate, dividend_yield, volatility, time_to_expiry, steps, type, american, Span<Real>{workspace.data(), workspace.size()}, output);
    if (!status) return status;
    return output;
}

} // namespace qf::pricing
