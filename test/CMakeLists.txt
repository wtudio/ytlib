# ytlib开发测试专用

add_subdirectory(custom_bench)

add_subdirectory(testlib)
add_subdirectory(testlib2)
add_subdirectory(testprj)

add_subdirectory(testytrpc)

add_subdirectory(testexecutor)
add_subdirectory(testboostfiber)
add_subdirectory(testboostasio)

if(WIN32)
  add_subdirectory(testgame)
  add_subdirectory(testgl)
endif()
