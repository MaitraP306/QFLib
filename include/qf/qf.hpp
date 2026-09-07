#pragma once

#include <string_view>

#include <qf/core.hpp>
#include <qf/math.hpp>
#include <qf/finance.hpp>
#include <qf/linalg.hpp>

namespace qf {

[[nodiscard]] constexpr std::string_view version() noexcept {
    return "0.1.0";
}

} // namespace qf

#include <qf/stats.hpp>
#include <qf/timeseries.hpp>
#include <qf/stochastic.hpp>
