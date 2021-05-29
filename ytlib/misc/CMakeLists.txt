# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置源文件
File(GLOB_RECURSE src *.cpp *.hpp *.h)
File(GLOB_RECURSE test_files *_test.cpp)
File(GLOB_RECURSE benchmark_files *_benchmark.cpp)
LIST(REMOVE_ITEM src ${test_files} ${benchmark_files})
SOURCE_GROUP(${CUR_TARGET_NAME} FILES ${src})

# 生成项目
# add_library(${CUR_TARGET_NAME} 
#   ${src}
# )

# 添加依赖
# add_dependencies(${CUR_TARGET_NAME} xxx)
# target_link_libraries(${CUR_TARGET_NAME} xxx)

# 其他设置
# set_misc_for_target(${CUR_TARGET_NAME})

# 设置测试
add_gtest_for_hpp(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${test_files} DEP_LIB dl)
add_benchmark_test_for_hpp(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${benchmark_files} DEP_LIB dl)
