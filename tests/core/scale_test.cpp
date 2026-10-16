#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <type_traits>

#include <qf/finance/types.hpp>

namespace {

using namespace qf;
using namespace qf::finance;

void test_representation(){
    static_assert(std::is_same_v<ScaledPrice::value_type, Int>);
    static_assert(std::is_same_v<ScaledPriceDelta::value_type, Int>);
    static_assert(std::is_same_v<ScaledQuantity::value_type, Int>);
    static_assert(std::is_same_v<ScaledNotional::value_type, Int>);
    static_assert(std::is_same_v<Scale::value_type, Int>);

    static_assert(sizeof(ScaledPrice) == sizeof(Int));
    static_assert(sizeof(ScaledPriceDelta) == sizeof(Int));
    static_assert(sizeof(ScaledQuantity) == sizeof(Int));
    static_assert(sizeof(ScaledNotional) == sizeof(Int));
    static_assert(sizeof(Scale) == sizeof(Int));

    static_assert(std::is_trivially_copyable_v<ScaledPrice>);
    static_assert(std::is_trivially_copyable_v<ScaledPriceDelta>);
    static_assert(std::is_trivially_copyable_v<ScaledQuantity>);
    static_assert(std::is_trivially_copyable_v<ScaledNotional>);
    static_assert(std::is_trivially_copyable_v<Scale>);

    static_assert(std::is_standard_layout_v<ScaledPrice>);
    static_assert(std::is_standard_layout_v<ScaledPriceDelta>);
    static_assert(std::is_standard_layout_v<ScaledQuantity>);
    static_assert(std::is_standard_layout_v<ScaledNotional>);
    static_assert(std::is_standard_layout_v<Scale>);

    assert(ScaledPrice{}.value() == 0);
    assert(ScaledPriceDelta{}.value() == 0);
    assert(ScaledQuantity{}.value() == 0);
    assert(ScaledNotional{}.value() == 0);
    assert(Scale{}.value() == 0);
}


void test_construction(){
    constexpr ScaledPrice price{12345};
    constexpr ScaledPriceDelta delta{-25};
    constexpr ScaledQuantity quantity{100};
    constexpr ScaledNotional notional{1234500};
    constexpr Scale scale{100};

    static_assert(price.value() == 12345);
    static_assert(delta.value() == -25);
    static_assert(quantity.value() == 100);
    static_assert(notional.value() == 1234500);
    static_assert(scale.value() == 100);
}


void test_price_arithmetic(){
    constexpr ScaledPrice p1{10000};
    constexpr ScaledPrice p2{10250};

    constexpr auto delta = p2 - p1;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(delta)>, ScaledPriceDelta>);

    static_assert(delta.value() == 250);

    constexpr auto p3 = p1 + delta;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(p3)>, ScaledPrice>);

    static_assert(p3.value() == 10250);

    constexpr auto p4 = delta + p1;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(p4)>, ScaledPrice>);

    static_assert(p4.value() == 10250);

    constexpr auto p5 = p2 - delta;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(p5)>, ScaledPrice>);

    static_assert(p5.value() == 10000);

    constexpr auto negative_delta = p1 - p2;

    static_assert(negative_delta.value() == -250);

    constexpr auto reconstructed = p1 + (p2 - p1);

    static_assert(reconstructed.value() == p2.value());
}


void test_quantity_arithmetic(){
    constexpr ScaledQuantity q1{100};
    constexpr ScaledQuantity q2{25};

    constexpr auto q3 = q1 + q2;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(q3)>,ScaledQuantity>);

    static_assert(q3.value() == 125);

    constexpr auto q4 = q1 - q2;

    static_assert(q4.value() == 75);

    constexpr ScaledQuantity short_quantity{-100};

    static_assert(short_quantity.value() == -100);

    constexpr auto positive_quantity = -short_quantity;

    static_assert(positive_quantity.value() == 100);
}


void test_notional_arithmetic(){
    constexpr ScaledPrice price{12500};
    constexpr ScaledQuantity quantity{100};
    constexpr auto notional = price * quantity;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(notional)>,ScaledNotional>);

    static_assert(notional.value() == 1'250'000);

    constexpr auto notional2 = quantity * price;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(notional2)>,ScaledNotional>);

    static_assert(notional2.value() == 1'250'000);

    assert(notional.value() == notional2.value());
}


void test_price_delta_times_quantity(){
    constexpr ScaledPriceDelta delta{250};
    constexpr ScaledQuantity quantity{100};

    constexpr auto pnl_like = delta * quantity;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(pnl_like)>,ScaledPnL>);

    static_assert(pnl_like.value() == 25'000);

    constexpr auto pnl_like2 = quantity * delta;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(pnl_like2)>,ScaledPnL>);

    static_assert(pnl_like2.value() == 25'000);

    assert(pnl_like.value() == pnl_like2.value());
}



void test_scale_conversion(){

    constexpr ScaledPrice raw_price{12345};
    constexpr Scale price_scale{100};

    constexpr auto price = raw_price / price_scale;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(price)>,Price>);

    static_assert(price.value() == 123.45);

    constexpr ScaledPriceDelta raw_delta{25};

    constexpr auto delta = raw_delta / price_scale;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(delta)>,PriceDelta>);

    static_assert(delta.value() == 0.25);
    constexpr ScaledNotional raw_notional{3'086'250};

    constexpr auto notional = raw_notional / price_scale;

    static_assert(std::is_same_v<std::remove_cv_t<decltype(notional)>,Notional>);

    static_assert(notional.value() == 30'862.5);
}


void test_abs(){
    constexpr ScaledPrice positive_price{100};
    constexpr ScaledPrice negative_price{-100};

    constexpr auto abs_positive = positive_price.abs();
    constexpr auto abs_negative = negative_price.abs();

    static_assert(abs_positive.value() == 100);
    static_assert(abs_negative.value() == 100);

    constexpr ScaledPriceDelta positive_delta{50};
    constexpr ScaledPriceDelta negative_delta{-50};

    constexpr auto abs_delta_positive = positive_delta.abs();
    constexpr auto abs_delta_negative = negative_delta.abs();

    static_assert(abs_delta_positive.value() == 50);
    static_assert(abs_delta_negative.value() == 50);

    constexpr ScaledQuantity negative_quantity{-200};

    constexpr auto abs_quantity = negative_quantity.abs();

    static_assert(abs_quantity.value() == 200);
}


void test_validation(){
    assert(valid(ScaledPrice{100}));
    assert(!valid(ScaledPrice{0}));
    assert(!valid(ScaledPrice{-100}));

    assert(valid(Scale{1}));
    assert(valid(Scale{100}));
    assert(!valid(Scale{0}));
    assert(!valid(Scale{-1}));

    assert(valid(ScaledPriceDelta{100}));
    assert(valid(ScaledQuantity{100}));
    assert(valid(ScaledNotional{100}));
}






void test_runtime_arithmetic(){
    const ScaledPrice price{12'345};
    const ScaledPriceDelta delta{-345};
    const auto result = price + delta;
    assert(result.value() == 12'000);
    const ScaledQuantity quantity{250};
    const auto notional = result * quantity;
    assert(notional.value() == 3'000'000);
    const Scale scale{100};
    const auto real_price = result / scale;
    assert(real_price.value() == 120.0);
    const auto real_notional = notional / scale;
    assert(real_notional.value() == 30'000.0);
}

} // namespace

int main()
{
    std::cout << "Running QFLib finance tests...\n";

    test_representation();
    std::cout << "  [PASS] representation\n";

    test_construction();
    std::cout << "  [PASS] construction\n";

    test_price_arithmetic();
    std::cout << "  [PASS] price arithmetic\n";

    test_quantity_arithmetic();
    std::cout << "  [PASS] quantity arithmetic\n";

    test_notional_arithmetic();
    std::cout << "  [PASS] notional arithmetic\n";

    test_price_delta_times_quantity();
    std::cout << "  [PASS] price delta * quantity\n";

    test_scale_conversion();
    std::cout << "  [PASS] scale conversion\n";

    test_abs();
    std::cout << "  [PASS] abs\n";

    test_validation();
    std::cout << "  [PASS] validation\n";

    test_runtime_arithmetic();
    std::cout << "  [PASS] runtime arithmetic\n";

    std::cout << "\nAll finance tests passed.\n";

    return 0;
}