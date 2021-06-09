cmake_minimum_required(VERSION 3.10)#最低版本要求
#cmake_policy设置（如果有）

project(ytlib)#工程名称

message(STATUS "start cmake ytlib...")

include(${ytlib_SOURCE_DIR}/CMake/util.cmake)

#定义变量，添加宏定义
option(YTLIB_UNICODE "build ytlib with unicode" OFF)
if(YTLIB_UNICODE)
  ADD_DEFINITIONS(-DUNICODE)
endif()

#设定c++版本
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#要导出给引用者的变量
set(YTLIB_LIBRARY)                          # ytlib自己生成的库
set(YTLIB_LIBRARY_DEPEND)                   # ytlib依赖的库
set(YTLIB_INCLUDE_DIR ${ytlib_SOURCE_DIR})   # ytlib头文件目录
set(YTLIB_LIBRARY_DIR)                      # ytlib库文件目录

#平台特化
if(WIN32)
  add_compile_options(/wd4819)
  set(BOOST_COMPONENTS)
else()
  set(YTLIB_LIBRARY_DEPEND ${YTLIB_LIBRARY_DEPEND} dl)
  set(BOOST_COMPONENTS system date_time filesystem thread log serialization)
endif()

#依赖库
find_package(Boost COMPONENTS ${BOOST_COMPONENTS})
if(NOT Boost_FOUND)
  message(FATAL_ERROR "Cannot find Boost library. please set the Boost_INCLUDE_DIRS to boost root path.")
endif()
mark_as_advanced(Boost_INCLUDE_DIRS Boost_LIBRARY_DIRS)
set(YTLIB_LIBRARY_DEPEND ${YTLIB_LIBRARY_DEPEND} ${Boost_LIBRARIES})
set(YTLIB_INCLUDE_DIR ${YTLIB_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
set(YTLIB_LIBRARY_DIR ${YTLIB_LIBRARY_DIR} ${Boost_LIBRARY_DIRS})

#设置cmake目录
list(APPEND CMAKE_MODULE_PATH ${ytlib_SOURCE_DIR}/CMake)

# 设置输出路径
set(YTLIB_OUTPUT_PATH ${ytlib_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${YTLIB_OUTPUT_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${YTLIB_OUTPUT_PATH})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${YTLIB_OUTPUT_PATH})
set(YTLIB_LIBRARY_DIR ${YTLIB_LIBRARY_DIR} ${YTLIB_OUTPUT_PATH})

#设置ide里文件目录
set(CMAKE_USE_FOLDERS ON CACHE INTERNAL "Use folder view in IDE")
if(CMAKE_MAKE_PROGRAM MATCHES ".?VCExpress.?")
  set(CMAKE_USE_FOLDERS OFF CACHE INTERNAL "Use folder view in IDE")
endif()
set_property(GLOBAL PROPERTY USE_FOLDERS ${CMAKE_USE_FOLDERS})

#设置默认编译类型
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

#设置库类型
option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)
if(BUILD_SHARED_LIBS)
  ADD_DEFINITIONS(-DBUILD_SHARED_LIBS)
endif()

#设置测试
option(BUILD_TESTS "Build tests." OFF)
if(BUILD_TESTS)
  set(googletest_ROOT_DIR "googletest_ROOT_DIR-NOTFOUND" CACHE PATH "path to gtest install path.")
  find_package(googletest QUIET)
  if(NOT googletest_FOUND)
    message(FATAL_ERROR "Cannot find googletest library. please set the googletest_ROOT_DIR to googletest install path.")
  endif()
  mark_as_advanced(googletest_ROOT_DIR)
  set(YTLIB_INCLUDE_DIR ${YTLIB_INCLUDE_DIR} ${googletest_INCLUDE_DIRS})
  enable_testing()
endif()

#设置benchmark测试
option(BUILD_BENCH_TESTS "Build benchmark tests." OFF)
if(BUILD_BENCH_TESTS)
  set(benchmark_ROOT_DIR "benchmark_ROOT_DIR-NOTFOUND" CACHE PATH "path to googlebenchmark install path.")
  find_package(benchmark QUIET)
  if(NOT benchmark_FOUND)
    message(FATAL_ERROR "Cannot find googlebenchmark library. please set the benchmark_ROOT_DIR to googlebenchmark install path.")
  endif()
  mark_as_advanced(benchmark_ROOT_DIR benchmark_DIR)
  set(YTLIB_INCLUDE_DIR ${YTLIB_INCLUDE_DIR} ${benchmark_INCLUDE_DIRS})
endif()

#链接设置
include_directories(${YTLIB_INCLUDE_DIR})
link_directories(${YTLIB_LIBRARY_DIR})

#需要编译的库
set(YTLIB_LIBRARY ${YTLIB_LIBRARY} 
  log
  math_tools
  string_tools
)

#设置下一级目录
add_subdirectory(ytlib/boost_asio)
add_subdirectory(ytlib/boost_tools)
add_subdirectory(ytlib/container)
add_subdirectory(ytlib/file_tools)
add_subdirectory(ytlib/log)
add_subdirectory(ytlib/math_tools)
add_subdirectory(ytlib/misc)
add_subdirectory(ytlib/string_tools)
add_subdirectory(ytlib/thread_tools)

if(BUILD_TESTS)
  add_subdirectory(ytlib/custom_test)
endif()

#提供给使用者的配置文件config.in
configure_file(${ytlib_SOURCE_DIR}/CMake/UseYTLIB.cmake.in  ${ytlib_BINARY_DIR}/UseYTLIB.cmake @ONLY)

#导出文档
set(ytlib_DOC_DIR ${ytlib_SOURCE_DIR}/doc)
if(WIN32)
  execute_process(COMMAND ${ytlib_DOC_DIR}/buildDoc.bat ${ytlib_DOC_DIR} WORKING_DIRECTORY ${ytlib_DOC_DIR})
else()
  execute_process(COMMAND ${ytlib_DOC_DIR}/buildDoc.sh ${ytlib_DOC_DIR} WORKING_DIRECTORY ${ytlib_DOC_DIR})
endif()
