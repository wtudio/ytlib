# ytlib开发测试专用

add_subdirectory(custom_bench)
add_subdirectory(testlib)
add_subdirectory(testprj)

if(YTLIB_BUILD_WITH_PROTOBUF)
  add_subdirectory(testprotobuf)
endif()
