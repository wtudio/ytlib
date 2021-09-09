# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置文件集合
File(GLOB_RECURSE head_files ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
File(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)
File(GLOB_RECURSE benchmark_files ${CMAKE_CURRENT_SOURCE_DIR}/*_benchmark.cpp)

# 添加目标
add_library(${CUR_TARGET_NAME} INTERFACE)
add_library(ytlib::${CUR_TARGET_NAME} ALIAS ${CUR_TARGET_NAME})

# 设置头文件路径
target_include_directories(${CUR_TARGET_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../>)

# 设置头文件
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY PUBLIC_HEADER ${head_files})

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
  add_gtest(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${test_files} DEP_LIB ytlib::misc)
endif()

if (YTLIB_BUILD_BENCH_TESTS)
#   add_benchmark_test(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${benchmark_files})
endif()

# 设置安装
install(TARGETS ${CUR_TARGET_NAME} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ytlib/${CUR_DIR})
