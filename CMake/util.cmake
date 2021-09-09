# 为目标添加gtest测试项目
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

# 为目标添加benchmark测试项目
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
