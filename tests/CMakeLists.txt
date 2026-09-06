add_executable(qflib_tests
    core/smoke_test.cpp
)

target_link_libraries(qflib_tests
    PRIVATE
        qf::qflib
)

add_test(NAME qflib_tests COMMAND qflib_tests)

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
