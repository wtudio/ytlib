# ytlib开发测试专用

add_subdirectory(testlib)
add_subdirectory(testprj)

if(YTLIB_BUILD_BENCH_TESTS)
  add_subdirectory(custom_bench)
endif()
