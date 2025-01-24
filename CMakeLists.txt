cmake_minimum_required(VERSION 3.24)

project(ytlib LANGUAGES C CXX)

# Set cmake path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Some option
option(YTLIB_BUILD_DOC "Build ytlib document." OFF)
option(YTLIB_BUILD_TESTS "Build tests." OFF)
option(YTLIB_BUILD_BENCH_TESTS "Build benchmark tests." OFF)
option(YTLIB_BUILD_WITH_BOOST "Build with boost." OFF)
option(YTLIB_BUILD_WITH_PROTOBUF "Build with protobuf." OFF)
option(YTLIB_BUILD_WITH_LIBUNIFEX "Build with libunifex." OFF)
option(YTLIB_BUILD_WITH_STDEXEC "Build with stdexec." OFF)
option(YTLIB_BUILD_WITH_TBB "Build with tbb." OFF)
option(YTLIB_BUILD_CUSTOM_TESTS "Build custom tests." OFF)

# Some necessary settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(INSTALL_CONFIG_NAME ${PROJECT_NAME}-config)

# Master project configuration
if(NOT DEFINED YTLIB_MASTER_PROJECT)
  set(YTLIB_MASTER_PROJECT OFF)
  if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(YTLIB_MASTER_PROJECT ON)
  endif()
endif()

if(YTLIB_MASTER_PROJECT)
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

  if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        Release
        CACHE STRING "Choose the type of build.")
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
  endif()

  set_directory_properties(PROPERTIES COMPILE_DEFINITIONS $<$<CONFIG:Debug>:_DEBUG>)

  option(BUILD_SHARED_LIBS "Build with shared libraries." OFF)

  if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif()

  set_property(GLOBAL PROPERTY POSITION_INDEPENDENT_CODE ON)

  if(MSVC)
    add_compile_options(/utf-8 /wd4819)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
  endif()

  if(WIN32)
    add_compile_definitions(NOMINMAX)
  endif()

  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

# Build document
if(YTLIB_BUILD_DOC)
  message(STATUS "gen document ...")
  set(YTLIB_DOC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/document)
  if(WIN32)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.bat ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  elseif(UNIX)
    execute_process(COMMAND ${YTLIB_DOC_DIR}/buildDoc.sh ${YTLIB_DOC_DIR} WORKING_DIRECTORY ${YTLIB_DOC_DIR})
  endif()
endif()

# Include cmake module
include(NamespaceTool)

if(YTLIB_BUILD_TESTS)
  include(GetGTest)
  enable_testing()
endif()

if(YTLIB_BUILD_BENCH_TESTS)
  include(GetGoogleBenchmark)
endif()

if(YTLIB_BUILD_WITH_BOOST)
  include(GetBoost)
endif()

if(YTLIB_BUILD_WITH_PROTOBUF)
  include(GetProtoBuf)
  include(ProtobufGenCode)
endif()

if(YTLIB_BUILD_WITH_LIBUNIFEX)
  include(GetLibUnifex)
endif()

if(YTLIB_BUILD_WITH_STDEXEC)
  include(GetStdexec)
endif()

if(YTLIB_BUILD_WITH_TBB)
  include(GetTBB)
endif()

# Add subdirectory
add_subdirectory(ytlib)

if(YTLIB_BUILD_CUSTOM_TESTS)
  add_subdirectory(test)
endif()

# Install
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  DESTINATION ./
  FILES_MATCHING
  PATTERN "*.cmake")

install(EXPORT ${INSTALL_CONFIG_NAME} DESTINATION lib/cmake/${PROJECT_NAME})
