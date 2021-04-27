cmake_minimum_required(VERSION 3.1)#最低版本要求
#cmake_policy设置（如果有）

project( ytlib )#工程名称

include( ${ytlib_SOURCE_DIR}/CMake/util.cmake )

#定义变量，添加宏定义
option(YTLIB_UNICODE "build ytlib with unicode" OFF)
mark_as_advanced(YTLIB_UNICODE)
IF (YTLIB_UNICODE)
  ADD_DEFINITIONS(-DUNICODE)
ENDIF()

option(YTLIB_DOUBLE_PRECISION "build ytlib with double precision" ON)
mark_as_advanced(YTLIB_DOUBLE_PRECISION)
IF (YTLIB_DOUBLE_PRECISION)
  ADD_DEFINITIONS(-DUSE_DOUBLE_PRECISION)
ENDIF()

#判断大小端
include (TestBigEndian)
TEST_BIG_ENDIAN(IS_BIG_ENDIAN)
if(IS_BIG_ENDIAN)
  message(STATUS "BIG_ENDIAN")
  ADD_DEFINITIONS(-DBIG_ENDIAN)
else()
  message(STATUS "LITTLE_ENDIAN")
endif()

#平台特化
IF(WIN32)
set( BOOST_COMPONENTS )
ELSE()
set( CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++1z" )
set( BOOST_COMPONENTS system date_time filesystem thread log serialization)
ENDIF()

#要导出的变量
set( YTLIB_LIBRARY )
set( YTLIB_INCLUDE_DIR ${ytlib_SOURCE_DIR})
set( YTLIB_LIBRARY_DIR )

#依赖库
find_package( Boost COMPONENTS ${BOOST_COMPONENTS})
IF ( NOT Boost_FOUND )
  message(FATAL_ERROR "Cannot find Boost library. please set the boost path")
  mark_as_advanced( Boost_INCLUDE_DIRS Boost_LIBRARY_DIRS )
ENDIF()
set( YTLIB_LIBRARY ${YTLIB_LIBRARY} ${Boost_LIBRARIES})
set( YTLIB_INCLUDE_DIR ${YTLIB_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
set( YTLIB_LIBRARY_DIR ${YTLIB_LIBRARY_DIR} ${Boost_LIBRARY_DIRS})
message( STATUS "${YTLIB_LIBRARY}")
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
set( YTLIB_LIBRARY_DIR ${YTLIB_LIBRARY_DIR} ${LIBRARY_OUTPUT_PATH})

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

#链接设置
include_directories( ${YTLIB_INCLUDE_DIR} )
link_directories( ${YTLIB_LIBRARY_DIR} )

#设置下一级目录
#TEST:测试项目
add_subdirectory( ytlib/Test )

#提供给使用者的配置文件config.in
configure_file( ${ytlib_SOURCE_DIR}/CMake/UseYTLIB.cmake.in  ${ytlib_BINARY_DIR}/UseYTLIB.cmake @ONLY )

#导出文档
set( ytlib_DOC_DIR ${ytlib_SOURCE_DIR}/doc )
IF(WIN32)
execute_process(COMMAND ${ytlib_DOC_DIR}/buildDoc.bat ${ytlib_DOC_DIR} WORKING_DIRECTORY ${ytlib_DOC_DIR})
ELSE()
execute_process(COMMAND ${ytlib_DOC_DIR}/buildDoc.sh ${ytlib_DOC_DIR} WORKING_DIRECTORY ${ytlib_DOC_DIR})
ENDIF()
