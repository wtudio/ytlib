cmake_minimum_required(VERSION 3.20)

project(ytlib VERSION 0.1.0 LANGUAGES C CXX)

#是否为主项目
if (NOT DEFINED YTLIB_MASTER_PROJECT)
  set(YTLIB_MASTER_PROJECT OFF)
  if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(YTLIB_MASTER_PROJECT ON)
    message(STATUS "start cmake ytlib ...")
  endif ()
endif ()

#设置cmake工具
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

#设定c++版本
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#设置文档
option(YTLIB_BUILD_DOC "Build ytlib doc." OFF)
if(YTLIB_BUILD_DOC)
  message(STATUS "gen doc ...")
  set(YTLIB_DOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/doc)
  if(WIN32)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.bat ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  elseif(UNIX)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.sh ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  endif()
endif()

#为主项目时的一些开发配置
if(YTLIB_MASTER_PROJECT)

  #设置默认编译类型
  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
  endif()

  set_directory_properties(PROPERTIES COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG>)

  #设置动态库还是静态库
  option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)

  #设置杂项
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)

  if(MSVC)
    add_compile_options(/wd4819)
  endif()

endif()

#设置第三方库。使用 FETCHCONTENT_BASE_DIR 以设置下载目录
message(STATUS "start get third party libraries, patience ...")

option(YTLIB_BUILD_WITH_BOOST "Build with boost stage lib." OFF)
if(YTLIB_BUILD_WITH_BOOST)
  set(Boost_USE_STATIC_LIBS ON)
  find_package(Boost REQUIRED COMPONENTS date_time log serialization program_options)
endif()

option(YTLIB_BUILD_WITH_FREEGLUT "Build with freeglut lib." OFF)
if(YTLIB_BUILD_WITH_FREEGLUT)
  include(GetFreeglut)
endif()

option(YTLIB_BUILD_WITH_PROTOBUF "Build with protobuf lib." OFF)
if(YTLIB_BUILD_WITH_PROTOBUF)
  include(GetProtoBuf)
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
add_subdirectory(ytlib)

#设置个性化测试（ytlib开发测试专用）
option(YTLIB_BUILD_CUSTOM_TESTS "Build custom tests." OFF)
if(YTLIB_BUILD_CUSTOM_TESTS)
  add_subdirectory(test)
endif()
