# 为目标添加gtest项目
function(add_gtest_for_target)
    cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC" ${ARGN})
    if (BUILD_TESTS)
        set(target_test_name ${ARG_TEST_TARGET}_test)

        SOURCE_GROUP(${target_test_name} FILES ${ARG_TEST_SRC})

        add_executable(${target_test_name} 
            ${ARG_TEST_SRC}
        )

        add_dependencies(${target_test_name} ${ARG_TEST_TARGET})
        target_link_libraries(${target_test_name} ${ARG_TEST_TARGET}
            GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

        set_target_properties(${target_test_name} PROPERTIES UNITY_BUILD ON)
        SET_PROPERTY(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})

        add_test(NAME ${target_test_name} COMMAND $<TARGET_FILE:${target_test_name}>)
    endif()
endfunction()

# 为目标添加benchmark test项目
function(add_benchmark_test_for_target)
    cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC" ${ARGN})
    if (BUILD_BENCH_TESTS)
        set(target_test_name ${ARG_TEST_TARGET}_benchmark)

        SOURCE_GROUP(${target_test_name} FILES ${ARG_TEST_SRC})

        add_executable(${target_test_name} 
            ${ARG_TEST_SRC}
        )

        add_dependencies(${target_test_name} ${ARG_TEST_TARGET})
        target_link_libraries(${target_test_name} ${ARG_TEST_TARGET}
            benchmark::benchmark benchmark::benchmark_main)

        set_target_properties(${target_test_name} PROPERTIES UNITY_BUILD ON)
        SET_PROPERTY(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})

    endif()
endfunction()

# 为hpp类库添加gtest项目
function(add_gtest_for_hpp)
    cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC;DEP_LIB" ${ARGN})
    if (BUILD_TESTS)
        set(target_test_name ${ARG_TEST_TARGET}_test)

        SOURCE_GROUP(${target_test_name} FILES ${ARG_TEST_SRC})

        add_executable(${target_test_name} 
            ${ARG_TEST_SRC}
        )

        target_link_libraries(${target_test_name} ${ARG_DEP_LIB}
            GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

        set_target_properties(${target_test_name} PROPERTIES UNITY_BUILD ON)
        SET_PROPERTY(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})

        add_test(NAME ${target_test_name} COMMAND $<TARGET_FILE:${target_test_name}>)
    endif()
endfunction()

# 为hpp类库添加benchmark test项目
function(add_benchmark_test_for_hpp)
    cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC;DEP_LIB" ${ARGN})
    if (BUILD_BENCH_TESTS)
        set(target_test_name ${ARG_TEST_TARGET}_benchmark)

        SOURCE_GROUP(${target_test_name} FILES ${ARG_TEST_SRC})

        add_executable(${target_test_name} 
            ${ARG_TEST_SRC}
        )

        target_link_libraries(${target_test_name} ${ARG_DEP_LIB}
            benchmark::benchmark benchmark::benchmark_main)

        set_target_properties(${target_test_name} PROPERTIES UNITY_BUILD ON)
        SET_PROPERTY(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})

    endif()
endfunction()

# 为目标库设置杂项
function(set_misc_for_target target)
    set_target_properties(${target} PROPERTIES UNITY_BUILD ON)
    SET_PROPERTY(TARGET ${target} PROPERTY FOLDER ${target})
endfunction()
