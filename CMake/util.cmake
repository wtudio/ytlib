
# 定义__FILENAME__
function(define_filename_macro target)
    #获取当前目标的所有源文件
    get_target_property(src_files "${target}" SOURCES)
    #遍历源文件
    foreach(src_file ${src_files})
        #获取当前源文件的编译参数
        get_property(defs SOURCE "${src_file}" PROPERTY COMPILE_DEFINITIONS)
        #获取当前文件的绝对路径
        get_filename_component(filepath "${src_file}" ABSOLUTE)
        #将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
        string(REPLACE ${CMAKE_SOURCE_DIR}/ "" relpath ${filepath})
        #将我们要加的编译参数(__FILENAME__)添加到原来的编译参数里面
        list(APPEND defs "__FILENAME__=\"${relpath}\"")
        #重新设置源文件的编译参数
        set_property(
            SOURCE "${src_file}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
            )
    endforeach()
endfunction()

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
        define_filename_macro(${target_test_name})

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
        define_filename_macro(${target_test_name})
    endif()
endfunction()

# 为hpp类库添加gtest项目
function(add_gtest_for_hpp)
    cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC" ${ARGN})
    if (BUILD_TESTS)
        set(target_test_name ${ARG_TEST_TARGET}_test)

        SOURCE_GROUP(${target_test_name} FILES ${ARG_TEST_SRC})

        add_executable(${target_test_name} 
            ${ARG_TEST_SRC}
        )

        target_link_libraries(${target_test_name}
            GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

        set_target_properties(${target_test_name} PROPERTIES UNITY_BUILD ON)
        SET_PROPERTY(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})
        define_filename_macro(${target_test_name})

        add_test(NAME ${target_test_name} COMMAND $<TARGET_FILE:${target_test_name}>)
    endif()
endfunction()

# 为hpp类库添加benchmark test项目
function(add_benchmark_test_for_hpp)
    cmake_parse_arguments(ARG "" "TEST_TARGET" "TEST_SRC" ${ARGN})
    if (BUILD_BENCH_TESTS)
        set(target_test_name ${ARG_TEST_TARGET}_benchmark)

        SOURCE_GROUP(${target_test_name} FILES ${ARG_TEST_SRC})

        add_executable(${target_test_name} 
            ${ARG_TEST_SRC}
        )

        target_link_libraries(${target_test_name}
            benchmark::benchmark benchmark::benchmark_main)

        set_target_properties(${target_test_name} PROPERTIES UNITY_BUILD ON)
        SET_PROPERTY(TARGET ${target_test_name} PROPERTY FOLDER ${ARG_TEST_TARGET})
        define_filename_macro(${target_test_name})
    endif()
endfunction()

# 为目标库设置杂项
function(set_misc_for_target target)
    set_target_properties(${target} PROPERTIES UNITY_BUILD ON)
    SET_PROPERTY(TARGET ${target} PROPERTY FOLDER ${target})
    define_filename_macro(${target})
endfunction()
