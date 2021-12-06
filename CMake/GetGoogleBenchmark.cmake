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

# 引入的target：
# benchmark::benchmark
# benchmark::benchmark_main

# 为目标添加benchmark测试项目
# eg: add_benchmark_test(TEST_TARGET targetxxx TEST_SRC "xxx.h;xxx.cpp" DEP_LIB "libxxx;libyyy")
function(add_benchmark_test)
  cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC;DEP_LIB" ${ARGN})
  set(target_test_name ${ARG_TEST_TARGET}_benchmark)

  add_executable(${target_test_name} 
    ${ARG_TEST_SRC}
  )

  add_dependencies(${target_test_name} ${ARG_TEST_TARGET} ${ARG_DEP_LIB})
  target_link_libraries(${target_test_name} PRIVATE ${ARG_TEST_TARGET} ${ARG_DEP_LIB}
    benchmark::benchmark benchmark::benchmark_main)

  set_property(TARGET ${target_test_name} PROPERTY UNITY_BUILD ON)
  set_property(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})
endfunction()
