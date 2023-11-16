include(FetchContent)

message(STATUS "get boost ...")

FetchContent_Declare(
  boost
  URL https://github.com/boostorg/boost/releases/download/boost-1.82.0/boost-1.82.0.tar.xz
  DOWNLOAD_EXTRACT_TIMESTAMP ON
  OVERRIDE_FIND_PACKAGE)

FetchContent_GetProperties(boost)
if(NOT boost_POPULATED)
  FetchContent_Populate(boost)

  set(BOOST_INCLUDE_LIBRARIES
      beast
      asio
      date_time
      log
      serialization
      program_options
      fiber
      property_tree)
  set(Boost_USE_STATIC_LIBS
      ON
      CACHE BOOL "")

  # set(BOOST_SKIP_INSTALL_RULES OFF CACHE BOOL "")
  file(READ ${boost_SOURCE_DIR}/tools/cmake/include/BoostRoot.cmake TMP_VAR)
  string(REPLACE "set(BOOST_SKIP_INSTALL_RULES ON)" "set(BOOST_SKIP_INSTALL_RULES OFF)" TMP_OUT_VAR "${TMP_VAR}")
  file(WRITE ${boost_SOURCE_DIR}/tools/cmake/include/BoostRoot.cmake "${TMP_OUT_VAR}")

  add_subdirectory(${boost_SOURCE_DIR} ${boost_BINARY_DIR})
endif()
