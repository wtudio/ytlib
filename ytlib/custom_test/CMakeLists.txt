# 获取当前文件夹
STRING(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# 设置目标名称
SET(CUR_TARGET_NAME ${CUR_DIR})

# 设置源文件
File(GLOB_RECURSE src *.cpp *.hpp *.h)
SOURCE_GROUP(${CUR_TARGET_NAME} FILES ${src})

# 生成项目
add_executable( ${CUR_TARGET_NAME} 
  ${src}
)

# 添加依赖
add_dependencies(${CUR_TARGET_NAME} ${YTLIB_LIBRARY})
target_link_libraries(${CUR_TARGET_NAME} ${YTLIB_LIBRARY} ${YTLIB_LIBRARY_DEPEND})

# 其他设置
set_misc_for_target(${CUR_TARGET_NAME})
