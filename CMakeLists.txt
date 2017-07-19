cmake_minimum_required(VERSION 3.1)#最低版本要求

#cmake_policy设置（如果有）

project( ytlib )#工程名称

#定义变量，添加宏定义
option(YTLIB_BUILD_TEST "build tset project" ON)
mark_as_advanced(YTLIB_BUILD_TEST)

#依赖库
find_package( Boost REQUIRED )
include_directories( ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

#设置cmake目录（让cmake能够找到自定义cmake函数。如findpackage等）
list( APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ytlib/CMake )

# 设置输出路径
if( NOT LIBRARY_OUTPUT_PATH )
  set( LIBRARY_OUTPUT_PATH ${ytlib_BINARY_DIR}/bin CACHE INTERNAL
    "Single output directory for building all libraries." )
endif()
if( NOT EXECUTABLE_OUTPUT_PATH )
  set( EXECUTABLE_OUTPUT_PATH ${ytlib_BINARY_DIR}/bin CACHE INTERNAL
    "Single output directory for building all executables." )
endif()
mark_as_advanced( LIBRARY_OUTPUT_PATH EXECUTABLE_OUTPUT_PATH )

#设置ide里文件目录
set( CMAKE_USE_FOLDERS ON CACHE INTERNAL "Use folder view in IDE" )
if( CMAKE_MAKE_PROGRAM MATCHES ".?VCExpress.?" )
  set( CMAKE_USE_FOLDERS OFF CACHE INTERNAL "Use folder view in IDE" )
endif()
set_property( GLOBAL PROPERTY USE_FOLDERS ${CMAKE_USE_FOLDERS} )


#设置默认编译类型
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
  # Set the possible values of build type for cmake-gui
  set_property( CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo" )
endif()

#设置库类型
option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)
mark_as_advanced( BUILD_SHARED_LIBS )

#设置字符集类型。如果使用unicode则使用宽字符
option(BUILD_UNICODE_VERSION "Build with UNICODE." OFF)
if( BUILD_UNICODE_VERSION )
  add_definitions(-DUNICODE -D_UNICODE)
endif()
mark_as_advanced( BUILD_UNICODE_VERSION )



# Include directories
include_directories( ${ytlib_SOURCE_DIR} )


#设置下一级目录
add_subdirectory( ytlib/Common )

#TEST:测试项目
if(YTLIB_BUILD_TEST)
  add_subdirectory( ytlib/Test )
endif()


#提供给使用者的

#配置文件config.in

