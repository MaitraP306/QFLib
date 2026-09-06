add_executable(qflib_benchmarks
    microbench.cpp
)
target_link_libraries(qflib_benchmarks PRIVATE qf::qflib)
target_compile_features(qflib_benchmarks PRIVATE cxx_std_20)
