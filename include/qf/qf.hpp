#pragma once

#include <string_view>

#include <qf/core.hpp>
#include <qf/math.hpp>
#include <qf/finance.hpp>
#include <qf/linalg.hpp>

namespace qf {

[[nodiscard]] constexpr std::string_view version() noexcept {
    return "1.0.0";
}

} // namespace qf

#include <qf/stats.hpp>
#include <qf/timeseries.hpp>
#include <qf/stochastic.hpp>
#include <qf/optimization.hpp>
#include <qf/probability.hpp>
#include <qf/portfolio.hpp>
#include <qf/risk.hpp>
#include <qf/pricing.hpp>

#include <qf/market.hpp>
