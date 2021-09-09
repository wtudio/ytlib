# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置文件集合
File(GLOB_RECURSE src *.cpp *.hpp *.h)
File(GLOB_RECURSE test_files *_test.cpp)
File(GLOB_RECURSE benchmark_files *_benchmark.cpp)
LIST(REMOVE_ITEM src ${test_files} ${benchmark_files})

# 添加目标
add_library(${CUR_TARGET_NAME} INTERFACE)

# 设置源文件
target_sources(${CUR_TARGET_NAME} INTERFACE ${src})

# 设置头文件路径
target_include_directories(${CUR_TARGET_NAME} INTERFACE ${CMAKE_SOURCE_DIR})

# 设置依赖
# add_dependencies(${CUR_TARGET_NAME} xxx)

# 设置链接库
# target_link_libraries(${CUR_TARGET_NAME} INTERFACE xxx)

# 设置编译选项
# target_compile_definitions(${CUR_TARGET_NAME} INTERFACE xxx)

# 设置其他属性
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY FOLDER ${CUR_TARGET_NAME})

# 设置测试
if (YTLIB_BUILD_TESTS)
  add_gtest(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${test_files} DEP_LIB misc)
endif()

if (YTLIB_BUILD_BENCH_TESTS)
#   add_benchmark_test(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${benchmark_files})
endif()
