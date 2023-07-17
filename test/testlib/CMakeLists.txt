# Get the current folder name
string(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Set target name
set(CUR_TARGET_NAME ${CUR_DIR})

# Set file collection
file(GLOB_RECURSE head_files ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Add target，设置库类型SHARED/STATIC
add_library(${CUR_TARGET_NAME} SHARED)
add_library(ytlib::${CUR_TARGET_NAME} ALIAS ${CUR_TARGET_NAME})

# Set source file of target
target_sources(${CUR_TARGET_NAME} PRIVATE ${src})

# Set include path of target
target_include_directories(
  ${CUR_TARGET_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(
  ${CUR_TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set head files of target
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY PUBLIC_HEADER ${head_files})
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY PRIVATE_HEADER ${head_files})

# Set link libraries of target
target_link_libraries(
  ${CUR_TARGET_NAME}
  PRIVATE ytlib::dll_tools)

# Set compile definitions of target
# target_compile_definitions(${CUR_TARGET_NAME} PRIVATE xxx)
target_compile_definitions(${CUR_TARGET_NAME} PRIVATE ${CUR_TARGET_NAME}_EXPORTS BUILD_SHARED_LIBS)

set_property(TARGET ${CUR_TARGET_NAME} PROPERTY DEBUG_POSTFIX "d")
