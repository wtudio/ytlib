# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置文件集合
File(GLOB head_files ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
File(GLOB src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# 添加目标
add_executable(${CUR_TARGET_NAME})

# 设置proto文件代码生成，信息保存到testgencode这个target中
add_protobuf_gencode_target(
  TARGET_NAME testgencode
  PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}/protos
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
add_library(protobuf_gencode::testgencode ALIAS testgencode)

# 设置源文件
target_sources(${CUR_TARGET_NAME} PRIVATE ${src})

# 设置头文件路径
target_include_directories(${CUR_TARGET_NAME} PRIVATE 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/gen
)

# 设置头文件
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY PRIVATE_HEADER ${head_files})

# 设置依赖
# add_dependencies(${CUR_TARGET_NAME} xxx)

# 设置链接库
target_link_libraries(${CUR_TARGET_NAME} PRIVATE
  ytlib::misc
  protobuf::libprotobuf
  protobuf_gencode::testgencode
)

# 设置编译选项
# target_compile_definitions(${CUR_TARGET_NAME} PRIVATE xxx)

# 设置其他属性
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY UNITY_BUILD ON)
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY FOLDER custom_test/${CUR_TARGET_NAME})
