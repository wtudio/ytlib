include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY  https://github.com/google/googletest.git
  GIT_TAG         release-1.11.0
)
if(WIN32)
  set(gtest_force_shared_crt ON CACHE BOOL "")
endif()
set(INSTALL_GTEST OFF CACHE BOOL "")
FetchContent_MakeAvailable(googletest)

# 引入的target：
# GTest::gtest
# GTest::gtest_main
# GTest::gmock
# GTest::gmock_main

# 为目标添加gtest测试项目
# eg: add_gtest(TEST_TARGET targetxxx TEST_SRC "xxx.h;xxx.cpp" DEP_LIB "libxxx;libyyy")
function(add_gtest)
  cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC;DEP_LIB" ${ARGN})
  set(target_test_name ${ARG_TEST_TARGET}_test)

  add_executable(${target_test_name} 
    ${ARG_TEST_SRC}
  )

  add_dependencies(${target_test_name} ${ARG_TEST_TARGET} ${ARG_DEP_LIB})
  target_link_libraries(${target_test_name} PRIVATE ${ARG_TEST_TARGET} ${ARG_DEP_LIB}
    GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

  set_property(TARGET ${target_test_name} PROPERTY UNITY_BUILD ON)
  set_property(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})

  add_test(NAME ${target_test_name} COMMAND $<TARGET_FILE:${target_test_name}>)
endfunction()
