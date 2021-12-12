include(FetchContent)

message(STATUS "get googlebenchmark ...")

FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/v1.6.0.tar.gz
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "")

FetchContent_MakeAvailable(googlebenchmark)

# import targets：
# benchmark::benchmark
# benchmark::benchmark_main

# add xxxlib_benchmark target for xxxlib
function(add_benchmark_target)
  cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC;INC_DIR" ${ARGN})
  set(target_test_name ${ARG_TEST_TARGET}_benchmark)

  add_executable(${target_test_name} ${ARG_TEST_SRC})
  target_link_libraries(${target_test_name} PRIVATE ${ARG_TEST_TARGET}
    benchmark::benchmark benchmark::benchmark_main)

  set_property(TARGET ${target_test_name} PROPERTY UNITY_BUILD ON)
endfunction()
