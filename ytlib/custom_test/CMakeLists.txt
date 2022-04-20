# ytlib开发测试专用

add_subdirectory(testlib)
add_subdirectory(testprj)
add_subdirectory(testgl)
add_subdirectory(testgame)
add_subdirectory(testnetlogsvr)

if(YTLIB_BUILD_BENCH_TESTS)
  add_subdirectory(custom_bench)
endif()
