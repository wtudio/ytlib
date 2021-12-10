include(FetchContent)

message(STATUS "get gflags ...")

FetchContent_Declare(
  gflags
  URL https://github.com/gflags/gflags/archive/v2.2.2.tar.gz
)

set(BUILD_TESTING OFF CACHE BOOL "")

FetchContent_MakeAvailable(gflags)

# 引入的target：
# gflags::gflags
