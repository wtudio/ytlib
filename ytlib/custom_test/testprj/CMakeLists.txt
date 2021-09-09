# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置文件集合
File(GLOB_RECURSE src *.cpp *.hpp *.h)

# 添加目标
add_executable(${CUR_TARGET_NAME})

# 设置源文件
target_sources(${CUR_TARGET_NAME} PRIVATE ${src})

# 设置头文件路径
target_include_directories(${CUR_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置依赖
add_dependencies(${CUR_TARGET_NAME} ytlib::boost_asio)

# 设置链接库
target_link_libraries(${CUR_TARGET_NAME} PRIVATE ytlib::boost_asio)

# 设置编译选项
# target_compile_definitions(${CUR_TARGET_NAME} PRIVATE xxx)

# 设置其他属性
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY UNITY_BUILD ON)
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY FOLDER custom_test/${CUR_TARGET_NAME})
