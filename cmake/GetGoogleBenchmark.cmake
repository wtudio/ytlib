include(FetchContent)

message(STATUS "get googlebenchmark ...")

FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/v1.8.3.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_GetProperties(googlebenchmark)
if(NOT googlebenchmark_POPULATED)
  set(BENCHMARK_ENABLE_TESTING
      OFF
      CACHE BOOL "")
  set(BENCHMARK_ENABLE_GTEST_TESTS
      OFF
      CACHE BOOL "")
  set(BENCHMARK_ENABLE_INSTALL
      OFF
      CACHE BOOL "")

  FetchContent_MakeAvailable(googlebenchmark)
endif()

# import targets:
# benchmark::benchmark
# benchmark::benchmark_main

# add xxxlib_benchmark target for xxxlib
function(add_benchmark_target)
  cmake_parse_arguments(ARG "" "BENCH_TARGET" "BENCH_SRC;INC_DIR" ${ARGN})
  set(BENCH_TARGET_NAME ${ARG_BENCH_TARGET}_benchmark)

  get_target_property(BENCH_TARGET_TYPE ${ARG_BENCH_TARGET} TYPE)
  if(${BENCH_TARGET_TYPE} STREQUAL SHARED_LIBRARY)
    get_target_property(BENCH_TARGET_SOURCES ${ARG_BENCH_TARGET} SOURCES)
    add_executable(${BENCH_TARGET_NAME} ${ARG_BENCH_SRC} ${BENCH_TARGET_SOURCES})

    get_target_property(BENCH_TARGET_INCLUDE_DIRECTORIES ${ARG_BENCH_TARGET} INCLUDE_DIRECTORIES)
    target_include_directories(${BENCH_TARGET_NAME} PRIVATE ${ARG_INC_DIR} ${BENCH_TARGET_INCLUDE_DIRECTORIES})

    get_target_property(BENCH_TARGET_LINK_LIBRARIES ${ARG_BENCH_TARGET} LINK_LIBRARIES)
    target_link_libraries(${BENCH_TARGET_NAME} PRIVATE ${BENCH_TARGET_LINK_LIBRARIES} benchmark::benchmark benchmark::benchmark_main)
  else()
    add_executable(${BENCH_TARGET_NAME} ${ARG_BENCH_SRC})
    target_include_directories(${BENCH_TARGET_NAME} PRIVATE ${ARG_INC_DIR})
    target_link_libraries(${BENCH_TARGET_NAME} PRIVATE ${ARG_BENCH_TARGET} benchmark::benchmark benchmark::benchmark_main)
  endif()
endfunction()
