#pragma once

#include <qf/core/error.hpp>
#include <qf/core/types.hpp>

namespace qf::probability {

struct Normal {
    Real mean = 0.0;
    Real standard_deviation = 1.0;
};

struct Lognormal {
    Real log_mean = 0.0;
    Real log_standard_deviation = 1.0;
};

struct StudentT {
    Real degrees_of_freedom = 1.0;
};

[[nodiscard]] Result<Real> normal_pdf(Real x, Normal distribution = {}) noexcept;
[[nodiscard]] Result<Real> normal_cdf(Real x, Normal distribution = {}) noexcept;
[[nodiscard]] Result<Real> normal_quantile(Real p, Normal distribution = {}) noexcept;

[[nodiscard]] Result<Real> lognormal_pdf(Real x, Lognormal distribution = {}) noexcept;
[[nodiscard]] Result<Real> lognormal_cdf(Real x, Lognormal distribution = {}) noexcept;
[[nodiscard]] Result<Real> lognormal_quantile(Real p, Lognormal distribution = {}) noexcept;

[[nodiscard]] Result<Real> student_t_pdf(Real x, StudentT distribution) noexcept;
[[nodiscard]] Result<Real> student_t_cdf(Real x, StudentT distribution) noexcept;
[[nodiscard]] Result<Real> student_t_quantile(Real p, StudentT distribution) noexcept;

[[nodiscard]] Result<Real> regularized_beta(Real a, Real b, Real x) noexcept;

} // namespace qf::probability
