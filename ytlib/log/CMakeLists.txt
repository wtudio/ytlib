# 以当前文件夹设置目标名
string(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CUR_TARGET_NAME ${CUR_DIR})

# 设置源文件
File(GLOB_RECURSE src *.cpp *.hpp *.h )
source_group( ${CUR_TARGET_NAME} FILES ${src} )

# 动态库（SHARED）/静态库（STATIC）/跟随工程（不填）
add_library(${CUR_TARGET_NAME} 
    ${src}
)

# 添加依赖（如果有）
# target_link_libraries(${CUR_TARGET_NAME} xxx)
# add_dependencies(${CUR_TARGET_NAME} xxx)

# 其他设置
set_target_properties(${CUR_TARGET_NAME} PROPERTIES UNITY_BUILD ON)
SET_PROPERTY(TARGET ${CUR_TARGET_NAME} PROPERTY FOLDER "deplibs")

define_filename_macro(${CUR_TARGET_NAME})
