#include <qf/probability/distributions.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

#include <qf/core/constants.hpp>
#include <qf/math/scalar.hpp>

namespace qf::probability {
namespace {

constexpr Real sqrt_two = qf::constants::sqrt_two;
constexpr Real inv_sqrt_two_pi = 1.0 / qf::constants::sqrt_two_pi;

[[nodiscard]] bool finite_positive(Real x) noexcept { return std::isfinite(x) && x > 0.0; }
[[nodiscard]] bool valid_normal(Normal d) noexcept { return std::isfinite(d.mean) && finite_positive(d.standard_deviation); }
[[nodiscard]] bool valid_lognormal(Lognormal d) noexcept { return std::isfinite(d.log_mean) && finite_positive(d.log_standard_deviation); }
[[nodiscard]] bool valid_student(StudentT d) noexcept { return finite_positive(d.degrees_of_freedom); }

[[nodiscard]] Real standard_normal_quantile(Real p) noexcept {
    constexpr Real a1=-3.969683028665376e+01, a2=2.209460984245205e+02, a3=-2.759285104469687e+02, a4=1.383577518672690e+02, a5=-3.066479806614716e+01, a6=2.506628277459239e+00;
    constexpr Real b1=-5.447609879822406e+01, b2=1.615858368580409e+02, b3=-1.556989798598866e+02, b4=6.680131188771972e+01, b5=-1.328068155288572e+01;
    constexpr Real c1=-7.784894002430293e-03, c2=-3.223964580411365e-01, c3=-2.400758277161838e+00, c4=-2.549732539343734e+00, c5=4.374664141464968e+00, c6=2.938163982698783e+00;
    constexpr Real d1=7.784695709041462e-03, d2=3.224671290700398e-01, d3=2.445134137142996e+00, d4=3.754408661907416e+00;
    constexpr Real plow=0.02425, phigh=1.0-plow;
    Real x;
    if (p < plow) {
        const Real q = std::sqrt(-2.0 * std::log(p));
        x = (((((c1*q+c2)*q+c3)*q+c4)*q+c5)*q+c6) / ((((d1*q+d2)*q+d3)*q+d4)*q+1.0);
    } else if (p > phigh) {
        const Real q = std::sqrt(-2.0 * std::log(1.0-p));
        x = -(((((c1*q+c2)*q+c3)*q+c4)*q+c5)*q+c6) / ((((d1*q+d2)*q+d3)*q+d4)*q+1.0);
    } else {
        const Real q = p - 0.5;
        const Real r = q*q;
        x = (((((a1*r+a2)*r+a3)*r+a4)*r+a5)*r+a6)*q / (((((b1*r+b2)*r+b3)*r+b4)*r+b5)*r+1.0);
    }
    const Real err = 0.5 * std::erfc(-x / sqrt_two) - p;
    const Real pdf = inv_sqrt_two_pi * std::exp(-0.5*x*x);
    if (pdf > std::numeric_limits<Real>::min()) x -= err / pdf;
    return x;
}

[[nodiscard]] Real beta_continued_fraction(Real a, Real b, Real x) noexcept {
    constexpr int max_iter = 200;
    constexpr Real eps = 3.0e-14;
    constexpr Real fpmin = std::numeric_limits<Real>::min() / eps;
    Real qab=a+b, qap=a+1.0, qam=a-1.0;
    Real c=1.0;
    Real d=1.0-qab*x/qap;
    if (std::abs(d) < fpmin) d=fpmin;
    d=1.0/d;
    Real h=d;
    for (int m=1; m<=max_iter; ++m) {
        const Real m2=2.0*m;
        Real aa=m*(b-m)*x/((qam+m2)*(a+m2));
        d=1.0+aa*d; if (std::abs(d)<fpmin) d=fpmin;
        c=1.0+aa/c; if (std::abs(c)<fpmin) c=fpmin;
        d=1.0/d; h*=d*c;
        aa=-(a+m)*(qab+m)*x/((a+m2)*(qap+m2));
        d=1.0+aa*d; if (std::abs(d)<fpmin) d=fpmin;
        c=1.0+aa/c; if (std::abs(c)<fpmin) c=fpmin;
        d=1.0/d;
        const Real del=d*c; h*=del;
        if (std::abs(del-1.0) < eps) break;
    }
    return h;
}

} // namespace

Result<Real> normal_pdf(Real x, Normal d) noexcept {
    if (!std::isfinite(x) || !valid_normal(d)) return Status{ErrorCode::invalid_argument, "invalid normal distribution or input"};
    const Real z=(x-d.mean)/d.standard_deviation;
    return inv_sqrt_two_pi/d.standard_deviation*std::exp(-0.5*z*z);
}

Result<Real> normal_cdf(Real x, Normal d) noexcept {
    if (!std::isfinite(x) || !valid_normal(d)) return Status{ErrorCode::invalid_argument, "invalid normal distribution or input"};
    const Real z=(x-d.mean)/(d.standard_deviation*sqrt_two);
    return 0.5*std::erfc(-z);
}

Result<Real> normal_quantile(Real p, Normal d) noexcept {
    if (!(p>0.0 && p<1.0) || !valid_normal(d)) return Status{ErrorCode::invalid_argument, "probability must be in (0,1) and distribution valid"};
    return d.mean+d.standard_deviation*standard_normal_quantile(p);
}

Result<Real> lognormal_pdf(Real x, Lognormal d) noexcept {
    if (!(x>0.0) || !std::isfinite(x) || !valid_lognormal(d)) return Status{ErrorCode::invalid_argument, "invalid lognormal input"};
    const Real z=(std::log(x)-d.log_mean)/d.log_standard_deviation;
    return inv_sqrt_two_pi/(x*d.log_standard_deviation)*std::exp(-0.5*z*z);
}

Result<Real> lognormal_cdf(Real x, Lognormal d) noexcept {
    if (!(x>0.0) || !std::isfinite(x) || !valid_lognormal(d)) return Status{ErrorCode::invalid_argument, "invalid lognormal input"};
    const Real z=(std::log(x)-d.log_mean)/(d.log_standard_deviation*sqrt_two);
    return 0.5*std::erfc(-z);
}

Result<Real> lognormal_quantile(Real p, Lognormal d) noexcept {
    if (!(p>0.0 && p<1.0) || !valid_lognormal(d)) return Status{ErrorCode::invalid_argument, "invalid probability or lognormal distribution"};
    return std::exp(d.log_mean+d.log_standard_deviation*standard_normal_quantile(p));
}

Result<Real> regularized_beta(Real a, Real b, Real x) noexcept {
    if (!finite_positive(a) || !finite_positive(b) || !(x>=0.0 && x<=1.0) || !std::isfinite(x))
        return Status{ErrorCode::invalid_argument, "invalid beta parameters"};
    if (x==0.0) return 0.0;
    if (x==1.0) return 1.0;
    const Real log_beta=std::lgamma(a)+std::lgamma(b)-std::lgamma(a+b);
    const Real front=std::exp(a*std::log(x)+b*std::log1p(-x)-log_beta);
    if (x < (a+1.0)/(a+b+2.0)) return front*beta_continued_fraction(a,b,x)/a;
    return 1.0-front*beta_continued_fraction(b,a,1.0-x)/b;
}

Result<Real> student_t_pdf(Real x, StudentT d) noexcept {
    if (!std::isfinite(x) || !valid_student(d)) return Status{ErrorCode::invalid_argument, "invalid Student-t input"};
    const Real nu=d.degrees_of_freedom;
    const Real log_norm=std::lgamma((nu+1.0)*0.5)-std::lgamma(nu*0.5)-0.5*(std::log(nu)+std::log(2.0*qf::constants::pi));
    return std::exp(log_norm-0.5*(nu+1.0)*std::log1p(x*x/nu));
}

Result<Real> student_t_cdf(Real x, StudentT d) noexcept {
    if (!std::isfinite(x) || !valid_student(d)) return Status{ErrorCode::invalid_argument, "invalid Student-t input"};
    if (x==0.0) return 0.5;
    const Real nu=d.degrees_of_freedom;
    const Real z=nu/(nu+x*x);
    auto ib=regularized_beta(nu*0.5,0.5,z);
    if (!ib) return ib.error();
    return x>0.0 ? 1.0-0.5*ib.value() : 0.5*ib.value();
}

Result<Real> student_t_quantile(Real p, StudentT d) noexcept {
    if (!(p>0.0 && p<1.0) || !valid_student(d)) return Status{ErrorCode::invalid_argument, "invalid probability or Student-t distribution"};
    if (p==0.5) return 0.0;
    const bool positive=p>0.5;
    const Real target=positive?p:1.0-p;
    Real lo=0.0, hi=1.0;
    for (int i=0;i<128;++i) {
        auto c=student_t_cdf(hi,d); if (!c) return c.error();
        if ((positive && c.value()>=target) || (!positive && c.value()<=target)) break;
        hi*=2.0;
        if (!std::isfinite(hi)) return Status{ErrorCode::numerical_failure,"Student-t quantile bracket overflow"};
    }
    for (int i=0;i<100;++i) {
        const Real mid=0.5*(lo+hi);
        auto c=student_t_cdf(mid,d); if (!c) return c.error();
        if (c.value()<target) lo=mid; else hi=mid;
    }
    const Real result=0.5*(lo+hi);
    return positive?result:-result;
}

} // namespace qf::probability
