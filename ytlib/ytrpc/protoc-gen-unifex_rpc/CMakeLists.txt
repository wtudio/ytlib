# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置文件集合
File(GLOB_RECURSE head_files ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
File(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# 添加目标
add_executable(${CUR_TARGET_NAME})
add_executable(ytlib::ytrpc::${CUR_TARGET_NAME} ALIAS ${CUR_TARGET_NAME})

# 设置源文件
target_sources(${CUR_TARGET_NAME} PRIVATE ${src})

# 设置头文件路径
target_include_directories(${CUR_TARGET_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 设置头文件
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY PRIVATE_HEADER ${head_files})

# 设置链接库
target_link_libraries(${CUR_TARGET_NAME} PRIVATE
  ytlib::string
  protobuf::libprotobuf
  protobuf::libprotoc
)

# 设置编译选项
# target_compile_definitions(${CUR_TARGET_NAME} PRIVATE xxx)

# 设置其他属性
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY FOLDER ytrpc/${CUR_TARGET_NAME})

include(${CMAKE_CURRENT_SOURCE_DIR}/UnifexRpcGenCode.cmake)
