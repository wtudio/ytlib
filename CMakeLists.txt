cmake_minimum_required(VERSION 3.20)

project(ytlib VERSION 0.1.0 LANGUAGES C CXX)
message(STATUS "start cmake ytlib ...")

#设置cmake工具
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
include(Util)

#设定c++版本
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#设置文档
option(YTLIB_BUILD_DOC "Build ytlib doc." OFF)
if(YTLIB_BUILD_DOC)
  set(YTLIB_DOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc)
  if(WIN32)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.bat ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  elseif(UNIX)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.sh ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  endif()
endif()

#设置默认编译类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#设置动态库还是静态库
option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)

#设置杂项
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)

if(MSVC)
  add_compile_options(/wd4819)
endif()

#设置是否编译依赖boost库的部分（boost静态库）
option(YTLIB_BUILD_WITH_BOOST "Build with boost stage lib." OFF)
if(YTLIB_BUILD_WITH_BOOST)
  set(Boost_USE_STATIC_LIBS ON)
  find_package(Boost REQUIRED COMPONENTS date_time log serialization program_options)
endif()

#设置第三方库
option(YTLIB_BUILD_WITH_PROTOBUF "Build with protobuf lib." OFF)
if(YTLIB_BUILD_WITH_PROTOBUF)
  include(GetProtoBuf)
endif()

option(YTLIB_BUILD_WITH_GFLAGS "Build with protobuf lib." OFF)
if(YTLIB_BUILD_WITH_GFLAGS)
  include(GetGFlags)
endif()

option(YTLIB_BUILD_WITH_SNAPPY "Build with snappy lib." OFF)
if(YTLIB_BUILD_WITH_SNAPPY)
  include(GetSnappy)
endif()

#设置测试
option(YTLIB_BUILD_TESTS "Build tests." OFF)
if(YTLIB_BUILD_TESTS)
  include(GetGTest)
  enable_testing()
endif()

#设置benchmark测试
option(YTLIB_BUILD_BENCH_TESTS "Build benchmark tests." OFF)
if(YTLIB_BUILD_BENCH_TESTS)
  include(GetGoogleBenchmark)
endif()

#设置下一级目录
add_subdirectory(ytlib/cache)
add_subdirectory(ytlib/container)
add_subdirectory(ytlib/dll_tools)
add_subdirectory(ytlib/file)
add_subdirectory(ytlib/log)
add_subdirectory(ytlib/logic)
add_subdirectory(ytlib/math)
add_subdirectory(ytlib/misc)
add_subdirectory(ytlib/string)
add_subdirectory(ytlib/thread)

if(YTLIB_BUILD_WITH_BOOST)
  add_subdirectory(ytlib/boost_asio)
  add_subdirectory(ytlib/boost_tools)
endif()

#设置个性化测试（ytlib开发测试专用）
option(YTLIB_BUILD_CUSTOM_TESTS "Build custom tests." OFF)
if(YTLIB_BUILD_CUSTOM_TESTS)
  add_subdirectory(ytlib/custom_test)
endif()
