# ytlib开发测试专用

add_subdirectory(testlib)
add_subdirectory(testprj)

if(YTLIB_BUILD_BENCH_TESTS)
  add_subdirectory(custom_bench)
endif()

if(YTLIB_BUILD_WITH_PROTOBUF)
  add_subdirectory(testprotobuf)
endif()

if(YTLIB_BUILD_WITH_GFLAGS)
  add_subdirectory(testgflags)
endif()

if(YTLIB_BUILD_WITH_SNAPPY)
  add_subdirectory(testsnappy)
endif()

if(YTLIB_BUILD_WITH_LEVELDB)
  add_subdirectory(testleveldb)
endif()
