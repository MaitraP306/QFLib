cmake_minimum_required(VERSION 3.20)

project(QFLib
    VERSION 0.1.0
    DESCRIPTION "High-performance quantitative finance and numerical library"
    LANGUAGES CXX
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(QFLibOptions)
include(QFLibWarnings)

option(QFLIB_BUILD_TESTS "Build QFLib tests" ON)
option(QFLIB_BUILD_BENCHMARKS "Build QFLib benchmarks" OFF)
option(QFLIB_BUILD_EXAMPLES "Build QFLib examples" ON)
option(QFLIB_ENABLE_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
option(QFLIB_ENABLE_LTO "Enable link-time optimization for QFLib" ON)
option(QFLIB_ENABLE_NATIVE_ARCH "Enable host-specific CPU instruction tuning" OFF)

add_library(qflib
    src/qf/math/scalar.cpp
    src/qf/math/reductions.cpp
    src/qf/math/polynomial.cpp
    src/qf/linalg/algorithms.cpp
    src/qf/stats/descriptive.cpp
    src/qf/stats/online.cpp
    src/qf/stats/covariance.cpp
    src/qf/stats/ols.cpp
    src/qf/stats/pca.cpp
    src/qf/timeseries/series.cpp
    src/qf/timeseries/operations.cpp
    src/qf/stochastic/ou.cpp
    src/qf/stochastic/processes.cpp
    src/qf/finance/analytics.cpp
    src/qf/probability/distributions.cpp
    src/qf/finance/portfolio.cpp
    src/qf/finance/risk.cpp
    src/qf/pricing/black_scholes.cpp
    src/qf/market/curve.cpp
    src/qf/optimization/L1_L2_regression.cpp
)

add_library(qf::qflib ALIAS qflib)

target_compile_features(qflib PUBLIC cxx_std_20)

target_include_directories(qflib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

qflib_set_warnings(qflib)

# Release builds should optimize the complete numerical kernel without using
# unsafe fast-math transformations that can invalidate financial edge cases.
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    target_compile_options(qflib PRIVATE
        $<$<CONFIG:Release>:-O3>
        $<$<CONFIG:Release>:-ffp-contract=fast>
    )
    if(QFLIB_ENABLE_NATIVE_ARCH)
        target_compile_options(qflib PRIVATE $<$<CONFIG:Release>:-march=native> $<$<CONFIG:Release>:-mtune=native>)
    endif()
endif()

if(QFLIB_ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT QFLIB_IPO_SUPPORTED OUTPUT QFLIB_IPO_ERROR LANGUAGES CXX)
    if(QFLIB_IPO_SUPPORTED)
        set_property(TARGET qflib PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "QFLib LTO/IPO unavailable: ${QFLIB_IPO_ERROR}")
    endif()
endif()

if(QFLIB_ENABLE_SANITIZERS)
    qflib_enable_sanitizers(qflib)
endif()

if(QFLIB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(QFLIB_BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif()

if(QFLIB_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

install(
    TARGETS qflib
    EXPORT QFLibTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(
    DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT QFLibTargets
    FILE QFLibTargets.cmake
    NAMESPACE qf::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QFLib
)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/QFLibConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/QFLibConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QFLib
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/QFLibConfigVersion.cmake
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/QFLibConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/QFLibConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/QFLib
)
