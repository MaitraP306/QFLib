add_executable(qflib_tests
    core/smoke_test.cpp
)

target_link_libraries(qflib_tests
    PRIVATE
        qf::qflib
)

add_test(NAME qflib_tests COMMAND qflib_tests)

add_executable(qflib_financial_type_tests
    core/financial_test.cpp
)
target_link_libraries(qflib_financial_type_tests PRIVATE qf::qflib)
add_test(NAME qflib_financial_type_tests COMMAND qflib_financial_type_tests)

add_executable(qflib_linalg_tests
    linalg/linalg_test.cpp
)

target_link_libraries(qflib_linalg_tests
    PRIVATE
        qf::qflib
)

add_test(NAME qflib_linalg_tests COMMAND qflib_linalg_tests)

add_executable(qflib_linalg_algorithm_tests linalg/algorithms_test.cpp)
target_link_libraries(qflib_linalg_algorithm_tests PRIVATE qf::qflib)
add_test(NAME qflib_linalg_algorithm_tests COMMAND qflib_linalg_algorithm_tests)

add_executable(qflib_stats_tests
    stats/stats_test.cpp
)

target_link_libraries(qflib_stats_tests
    PRIVATE
        qf::qflib
)

add_test(NAME qflib_stats_tests COMMAND qflib_stats_tests)

add_executable(qflib_pca_tests
    stats/pca_test.cpp
)
target_link_libraries(qflib_pca_tests PRIVATE qf::qflib)
add_test(NAME qflib_pca_tests COMMAND qflib_pca_tests)

add_executable(qflib_timeseries_tests
    timeseries_test.cpp
)
target_link_libraries(qflib_timeseries_tests PRIVATE qf::qflib)
add_test(NAME qflib_timeseries_tests COMMAND qflib_timeseries_tests)

add_executable(qflib_stochastic_tests
    stochastic/ou_test.cpp
)
target_link_libraries(qflib_stochastic_tests PRIVATE qf::qflib)
add_test(NAME qflib_stochastic_tests COMMAND qflib_stochastic_tests)

add_executable(qflib_financial_analytics_tests
    finance/analytics_test.cpp
)
target_link_libraries(qflib_financial_analytics_tests PRIVATE qf::qflib)
add_test(NAME qflib_financial_analytics_tests COMMAND qflib_financial_analytics_tests)

add_executable(qflib_online_stats_tests
    stats/online_test.cpp
)
target_link_libraries(qflib_online_stats_tests PRIVATE qf::qflib)
add_test(NAME qflib_online_stats_tests COMMAND qflib_online_stats_tests)

add_executable(qflib_financial_streaming_tests
    finance/streaming_test.cpp
)
target_link_libraries(qflib_financial_streaming_tests PRIVATE qf::qflib)
add_test(NAME qflib_financial_streaming_tests COMMAND qflib_financial_streaming_tests)

add_executable(qflib_optimization_tests
    optimization/optimization_test.cpp
)
target_link_libraries(qflib_optimization_tests PRIVATE qf::qflib)
add_test(NAME qflib_optimization_tests COMMAND qflib_optimization_tests)


add_executable(qflib_probability_tests
    probability/distributions_test.cpp
)
target_link_libraries(qflib_probability_tests PRIVATE qf::qflib)
add_test(NAME qflib_probability_tests COMMAND qflib_probability_tests)

add_executable(qflib_stochastic_process_tests
    stochastic/processes_test.cpp
)
target_link_libraries(qflib_stochastic_process_tests PRIVATE qf::qflib)
add_test(NAME qflib_stochastic_process_tests COMMAND qflib_stochastic_process_tests)

add_executable(qflib_portfolio_tests
    portfolio/portfolio_test.cpp
)
target_link_libraries(qflib_portfolio_tests PRIVATE qf::qflib)
add_test(NAME qflib_portfolio_tests COMMAND qflib_portfolio_tests)

add_executable(qflib_risk_tests
    risk/risk_test.cpp
)
target_link_libraries(qflib_risk_tests PRIVATE qf::qflib)
add_test(NAME qflib_risk_tests COMMAND qflib_risk_tests)

add_executable(qflib_pricing_tests
    pricing/pricing_test.cpp
)
target_link_libraries(qflib_pricing_tests PRIVATE qf::qflib)
add_test(NAME qflib_pricing_tests COMMAND qflib_pricing_tests)

add_executable(qflib_market_tests market/market_test.cpp)
target_link_libraries(qflib_market_tests PRIVATE qf::qflib)
add_test(NAME qflib_market_tests COMMAND qflib_market_tests)
