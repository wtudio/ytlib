include(FetchContent)

message(STATUS "get TBB ...")

set(tbb_DOWNLOAD_URL
    "https://github.com/oneapi-src/oneTBB/archive/v2021.10.0.tar.gz"
    CACHE STRING "")

FetchContent_Declare(
  tbb
  URL ${tbb_DOWNLOAD_URL}
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE)

FetchContent_GetProperties(tbb)
if(NOT tbb_POPULATED)
  set(TBB_TEST OFF)
  set(TBB_TEST
      OFF
      CACHE BOOL "")
  FetchContent_MakeAvailable(tbb)
endif()
