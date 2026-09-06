#include <cmath>
#include <cstdlib>
#include <iostream>

#include <qf/qf.hpp>

namespace {
void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAIL: " << message << '\n';
        std::exit(EXIT_FAILURE);
    }
}

void expect_close(double actual, double expected, double tolerance = 1e-12) {
    expect(std::abs(actual - expected) <= tolerance, "values differ beyond tolerance");
}
} // namespace

int main() {
    qf::linalg::Vector<> v{1.0, 2.0, 3.0};
    expect(v.size() == 3, "vector size");
    expect_close(v[1], 2.0);

    auto v2 = qf::linalg::Vector<>{4.0, 5.0, 6.0};
    auto sum = qf::linalg::add(v.span(), v2.span());
    expect(sum.has_value(), "vector add shape");
    expect_close(sum.value()[2], 9.0);

    qf::linalg::Matrix<> m{{1.0, 2.0, 3.0}, {4.0, 5.0, 6.0}};
    expect(m.rows() == 2 && m.cols() == 3, "matrix shape");
    expect_close(m(1, 2), 6.0);
    expect_close(m.row(0)[1], 2.0);

    auto view = qf::linalg::matrix_view(m);
    auto block = qf::linalg::submatrix(view, 0, 1, 2, 2);
    block(1, 0) = 50.0;
    expect_close(m(1, 1), 50.0);

    auto transposed = qf::linalg::transpose(m);
    expect(transposed.rows() == 3 && transposed.cols() == 2, "transpose shape");
    expect_close(transposed(1, 1), 50.0);

    qf::linalg::Matrix<> other{{1.0, 2.0}, {3.0, 4.0}};
    auto mismatch = qf::linalg::add(m, other);
    expect(!mismatch.has_value(), "matrix shape mismatch rejected");
    expect(mismatch.error().code() == qf::ErrorCode::invalid_argument,
           "matrix mismatch error code");

    std::cout << "PASS: QFLib linalg tests\n";
    return EXIT_SUCCESS;
}
