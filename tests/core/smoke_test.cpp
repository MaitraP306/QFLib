#include <cstdlib>
#include <iostream>
#include <limits>
#include <string_view>

#include <qf/qf.hpp>

namespace {

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    bool ok = true;

    ok &= check(qf::version() == std::string_view{"1.0.0"}, "version");
    ok &= check(qf::almost_equal(1.0, 1.0 + 1e-12), "almost_equal");
    ok &= check(!qf::almost_equal(1.0, 1.1), "almost_equal rejects distant values");
    ok &= check(qf::is_finite(1.0), "is_finite");
    ok &= check(qf::is_nan(std::numeric_limits<double>::quiet_NaN()), "is_nan");
    ok &= check(qf::is_infinite(std::numeric_limits<double>::infinity()), "is_infinite");
    ok &= check(qf::clamp(5.0, 0.0, 3.0) == 3.0, "clamp upper bound");
    ok &= check(qf::clamp(-1.0, 0.0, 3.0) == 0.0, "clamp lower bound");

    const double values[] = {1.0, 2.0, 3.0};
    auto span = qf::make_const_span(values);
    ok &= check(span.size() == 3, "const span size");
    ok &= check(span[1] == 2.0, "const span access");

    const auto dt = qf::Duration::from_milliseconds(250);
    const auto t0 = qf::Timestamp::from_nanoseconds(1'000'000'000);
    const auto t1 = t0 + dt;
    ok &= check((t1 - t0).count() == 250'000'000, "timestamp duration arithmetic");
    ok &= check(t1 > t0, "timestamp ordering");

    const qf::Status error{qf::ErrorCode::invalid_argument, "bad input"};
    qf::Result<int> result{error};
    ok &= check(!result, "error result state");
    ok &= check(result.error().code() == qf::ErrorCode::invalid_argument, "result error code");

    qf::Result<int> value_result{42};
    ok &= check(value_result && value_result.value() == 42, "value result state");

    ok &= check(qf::math::square(3.0) == 9.0, "square");
    ok &= check(qf::math::cube(-2.0) == -8.0, "cube");
    ok &= check(qf::math::sign(-2.0) == -1, "sign negative");
    ok &= check(qf::math::sign(0.0) == 0, "sign zero");
    ok &= check(qf::math::sign(2.0) == 1, "sign positive");
    ok &= check(qf::almost_equal(qf::math::lerp(2.0, 6.0, 0.25), 3.0), "lerp");
    ok &= check(qf::almost_equal(qf::math::hypot2(3.0, 4.0), 5.0), "hypot2");

    ok &= check(qf::almost_equal(qf::math::expm1(1e-8), 1e-8), "expm1");
    ok &= check(qf::almost_equal(qf::math::log1p(1e-8), 1e-8), "log1p");
    ok &= check(qf::almost_equal(qf::math::log1pexp(1000.0), 1000.0), "stable log1pexp");
    ok &= check(qf::almost_equal(qf::math::sigmoid(1000.0), 1.0), "stable sigmoid positive");
    ok &= check(qf::almost_equal(qf::math::sigmoid(-1000.0), 0.0), "stable sigmoid negative");

    const double math_values[] = {1.0, 2.0, 3.0, 4.0};
    const auto math_span = qf::make_const_span(math_values);
    ok &= check(qf::almost_equal(qf::math::sum(math_span), 10.0), "sum");
    ok &= check(qf::almost_equal(qf::math::sum_squares(math_span), 30.0), "sum_squares");
    ok &= check(qf::almost_equal(qf::math::mean(math_span), 2.5), "mean");
    ok &= check(qf::almost_equal(qf::math::rms(math_span), std::sqrt(7.5)), "rms");
    ok &= check(qf::math::min(math_span) == 1.0, "min");
    ok &= check(qf::math::max(math_span) == 4.0, "max");

    const double cancellation[] = {1e16, 1.0, -1e16};
    ok &= check(qf::math::sum(qf::make_const_span(cancellation)) == 1.0,
                "compensated sum");

    const double coefficients[] = {2.0, -3.0, 4.0};
    ok &= check(qf::almost_equal(qf::math::horner(3.0, qf::make_const_span(coefficients)), 13.0),
                "Horner polynomial evaluation");

    const qf::ConstSpan<double> empty{};
    ok &= check(std::isnan(qf::math::mean(empty)), "empty mean");
    ok &= check(std::isnan(qf::math::min(empty)), "empty min");


    std::cout << (ok ? "PASS: QFLib Phase 2 math tests\n" : "FAIL: QFLib Phase 2 math tests\n");
    return ok ? EXIT_SUCCESS : EXIT_FAILURE;
}
