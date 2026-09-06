#include <qf/math/polynomial.hpp>

namespace qf::math {

double horner(double x, ConstSpan<double> coefficients) noexcept {
    double result = 0.0;
    for (double coefficient : coefficients) result = result * x + coefficient;
    return result;
}

} // namespace qf::math
