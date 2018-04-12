cmake_minimum_required(VERSION 3.1)#最低版本要求

#cmake_policy设置（如果有）

project( ytlib )#工程名称

#定义变量，添加宏定义
option(YTLIB_BUILD_TEST "build tset project" ON)
mark_as_advanced(YTLIB_BUILD_TEST)

option(YTLIB_BUILD_RPSF_DEMO "build remote plugin service framework project" ON)
mark_as_advanced(YTLIB_BUILD_RPSF_DEMO)

#依赖库
find_package( Boost )
IF ( NOT Boost_FOUND)
	message(FATAL_ERROR "Cannot find Boost library. please set the boost path")
	mark_as_advanced( Boost_INCLUDE_DIRS Boost_LIBRARY_DIRS )
ENDIF()
include_directories( ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

#非必需库
find_package( OpenCV QUIET)
IF ( OpenCV_FOUND)
	message(STATUS "OpenCV library status:")
	message(STATUS "    version: ${OpenCV_VERSION}")
	message(STATUS "    libraries: ${OpenCV_LIBS}")
	message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
	include_directories( ${OpenCV_INCLUDE_DIRS} )
#target_link_libraries( xxx ${OpenCV_LIBS}  )
ELSE()
	message(STATUS "OpenCV library is not found.")
	mark_as_advanced( OpenCV_DIR )
ENDIF()


#设置cmake目录（让cmake能够找到自定义cmake函数。如findpackage等）
list( APPEND CMAKE_MODULE_PATH ${ytlib_SOURCE_DIR}/CMake )

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
#添加debug宏定义
IF (CMAKE_BUILD_TYPE STREQUAL Debug)
    ADD_DEFINITIONS(-DDEBUG)
ENDIF()

#设置库类型
option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)
mark_as_advanced( BUILD_SHARED_LIBS )


# Include directories
include_directories( ${ytlib_SOURCE_DIR} )


#设置下一级目录
add_subdirectory( sqlite )
add_subdirectory( sigar )
#TEST:测试项目
if(YTLIB_BUILD_TEST)
  add_subdirectory( ytlib/Test )
endif()
#ytrpsf：远程插件服务框架
if(YTLIB_BUILD_RPSF_DEMO)
  add_subdirectory( ytrpsf/RPSF_Demo/RPSF_CenterNode )
  add_subdirectory( ytrpsf/RPSF_Demo/RPSF_CommonNode )
  add_subdirectory( ytrpsf/RPSF_Demo/RPSF_LogServer )

  add_subdirectory( ytrpsf/RPSF_Demo/RPSF_DemoPlugin1 )
  add_subdirectory( ytrpsf/RPSF_Demo/RPSF_DemoPlugin2 )
  add_subdirectory( ytrpsf/RPSF_Demo/RPSF_DemoPlugin3 )
endif()
#基于opencv的项目
if ( OpenCV_FOUND)
  add_subdirectory( ytlib/Test_OpenCV )
endif()

#配置文件config.in
#提供给使用者的

configure_file( ${ytlib_SOURCE_DIR}/CMake/UseYTLIB.cmake.in  ${ytlib_BINARY_DIR}/UseYTLIB.cmake @ONLY )

