cmake_minimum_required(VERSION 3.10)

project(ytlib VERSION 0.1.0 LANGUAGES C CXX)
message(STATUS "start cmake ytlib ...")

#设置cmake工具
set(YTLIB_CMAKE_TOOL_PATH ${CMAKE_SOURCE_DIR}/CMake)
list(APPEND CMAKE_MODULE_PATH ${YTLIB_CMAKE_TOOL_PATH})
include(${YTLIB_CMAKE_TOOL_PATH}/util.cmake)
include(FetchContent)

#设定c++版本
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#设置文档
option(YTLIB_BUILD_DOC "Build ytlib doc." OFF)
if(YTLIB_BUILD_DOC)
  set(YTLIB_DOC_DIR ${CMAKE_SOURCE_DIR}/doc)
  if(WIN32)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.bat ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  elseif(UNIX)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.sh ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  endif()
endif()

#设置默认编译类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#设置动态库还是静态库
option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)
if(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_SHARED_LIBS)
endif()

#设置杂项
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)

if(WIN32)
  add_compile_options(/wd4819)
endif()

#要导出给引用者的变量
set(YTLIB_LIBRARY)                          # ytlib自己生成的库
set(YTLIB_LIBRARY_DEPEND)                   # ytlib依赖的库
set(YTLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR})  # ytlib头文件目录
set(YTLIB_LIBRARY_DIR)                      # ytlib库文件目录

if(UNIX)
  set(YTLIB_LIBRARY_DEPEND ${YTLIB_LIBRARY_DEPEND} dl)
endif()

# 设置输出路径
set(YTLIB_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${YTLIB_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${YTLIB_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${YTLIB_OUTPUT_PATH})
set(YTLIB_LIBRARY_DIR ${YTLIB_LIBRARY_DIR} ${YTLIB_OUTPUT_PATH})

#设置是否依赖boost库
option(BUILD_WITH_BOOST "Build with boost lib." OFF)
if(BUILD_WITH_BOOST)
  if(WIN32)
    find_package(Boost QUIET)
    if(NOT Boost_FOUND)
      message(FATAL_ERROR "Cannot find Boost library. please set the Boost_INCLUDE_DIRS to boost root path.")
    endif()
  else()
    find_package(Boost QUIET COMPONENTS log serialization)
    if(NOT Boost_FOUND)
      message(FATAL_ERROR "Cannot find Boost library. please set the Boost_INCLUDE_DIRS to boost root path.")
    endif()
    set(YTLIB_LIBRARY_DEPEND ${YTLIB_LIBRARY_DEPEND} ${Boost_LIBRARIES})
  endif()

  set(YTLIB_INCLUDE_DIR ${YTLIB_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
  set(YTLIB_LIBRARY_DIR ${YTLIB_LIBRARY_DIR} ${Boost_LIBRARY_DIRS})

endif()

#设置测试
option(BUILD_TESTS "Build tests." OFF)
if(BUILD_TESTS)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY  https://github.com/google/googletest.git
    GIT_TAG         release-1.11.0
  )
  if(WIN32)
    set(gtest_force_shared_crt ON CACHE BOOL "")
  endif()
  set(INSTALL_GTEST OFF CACHE BOOL "")
  FetchContent_MakeAvailable(googletest)
  enable_testing()
endif()

#设置benchmark测试
option(BUILD_BENCH_TESTS "Build benchmark tests." OFF)
if(BUILD_BENCH_TESTS)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY  https://github.com/google/benchmark.git
    GIT_TAG         v1.6.0
  )
  set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
  set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
  set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "")
  FetchContent_MakeAvailable(googlebenchmark)
endif()

#链接设置
include_directories(${YTLIB_INCLUDE_DIR})
link_directories(${YTLIB_LIBRARY_DIR})

#打印信息
message(STATUS "YTLIB_LIBRARY: ${YTLIB_LIBRARY}")
message(STATUS "YTLIB_LIBRARY_DEPEND: ${YTLIB_LIBRARY_DEPEND}")
message(STATUS "YTLIB_INCLUDE_DIR: ${YTLIB_INCLUDE_DIR}")
message(STATUS "YTLIB_LIBRARY_DIR: ${YTLIB_LIBRARY_DIR}")

#需要编译的库（保留，应朝head-only方向发展）
#set(YTLIB_LIBRARY ${YTLIB_LIBRARY} xxx )

#设置下一级目录
add_subdirectory(ytlib/cache)
add_subdirectory(ytlib/container)
add_subdirectory(ytlib/file)
add_subdirectory(ytlib/log)
add_subdirectory(ytlib/math)
add_subdirectory(ytlib/misc)
add_subdirectory(ytlib/string)
add_subdirectory(ytlib/thread)

if(BUILD_WITH_BOOST)
  add_subdirectory(ytlib/boost_asio)
  add_subdirectory(ytlib/boost_tools)
endif()

#设置个性化测试（ytlib开发测试专用）
option(BUILD_CUSTOM_TESTS "Build custom tests." OFF)
if(BUILD_CUSTOM_TESTS)
  add_subdirectory(ytlib/custom_test/custom_bench)
  add_subdirectory(ytlib/custom_test/testlib)
  add_subdirectory(ytlib/custom_test/testprj)
endif()

#提供给使用者的配置文件config.in
configure_file(${YTLIB_CMAKE_TOOL_PATH}/UseYTLIB.cmake.in  ${CMAKE_BINARY_DIR}/UseYTLIB.cmake @ONLY)
