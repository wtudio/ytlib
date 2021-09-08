# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置源文件
File(GLOB_RECURSE test_files *_test.cpp)
File(GLOB_RECURSE benchmark_files *_benchmark.cpp)

# 设置测试
if(UNIX)
  SET(TARGET_DEP_LIB dl)
endif()
add_gtest_for_hpp(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${test_files} DEP_LIB ${TARGET_DEP_LIB})
add_benchmark_test_for_hpp(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${benchmark_files} DEP_LIB ${TARGET_DEP_LIB})