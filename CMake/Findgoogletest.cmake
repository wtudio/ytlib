# Findgoogletest.cmake
# - Try to find googletest
#
# The following variables are optionally searched for defaults
#  googletest_ROOT_DIR:  Base directory where all googletest components are found
#
# Once done this will define
#  googletest_FOUND - System has googletest
#  googletest_INCLUDE_DIRS - The googletest include directories
#  googletest_LIBRARIES - The libraries needed to use googletest

set(googletest_ROOT_DIR "googletest_ROOT_DIR-NOTFOUND" CACHE PATH "Folder containing googletest")

find_path(gtest_INCLUDE_DIR "gtest/gtest.h"
  PATHS ${googletest_ROOT_DIR}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)

find_path(gmock_INCLUDE_DIR "gmock/gmock.h"
  PATHS ${googletest_ROOT_DIR}
  PATH_SUFFIXES include
  NO_DEFAULT_PATH)

find_library(gtest_LIBRARY NAMES "gtest"
  PATHS ${googletest_ROOT_DIR}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH)

find_library(gmock_LIBRARY NAMES "gmock"
  PATHS ${googletest_ROOT_DIR}
  PATH_SUFFIXES lib lib64
  NO_DEFAULT_PATH)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set googletest_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(googletest FOUND_VAR googletest_FOUND
  REQUIRED_VARS gtest_INCLUDE_DIR gmock_INCLUDE_DIR gtest_LIBRARY gmock_LIBRARY)

if(googletest_FOUND)
  set(googletest_INCLUDE_DIRS ${gtest_INCLUDE_DIR} ${gmock_INCLUDE_DIR})
  set(googletest_LIBRARIES ${gtest_LIBRARY} ${gmock_LIBRARY})
  include(${googletest_ROOT_DIR}/lib/cmake/GTest/GTestConfig.cmake)
endif()

mark_as_advanced(gtest_INCLUDE_DIR gmock_INCLUDE_DIR gtest_LIBRARY gmock_LIBRARY)
mark_as_advanced(googletest_INCLUDE_DIRS googletest_LIBRARIES)
