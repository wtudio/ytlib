# ytlib开发测试专用

include(GetHiRedis)
include(GetLibevent)
include(GetTBB)

add_subdirectory(log)

add_subdirectory(custom_bench)

add_subdirectory(testlib)
add_subdirectory(testlib2)
add_subdirectory(testprj)

# add_subdirectory(testytrpc)

add_subdirectory(testexecutor)
add_subdirectory(testboostfiber)
add_subdirectory(testboostasio)

add_subdirectory(testredis)

add_subdirectory(testthreadpool)

if(WIN32)
  include(GetFreeglut)

  add_subdirectory(testgame)
  add_subdirectory(testgl)
endif()
