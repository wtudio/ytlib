include(FetchContent)

FetchContent_Declare(
  snappy
  GIT_REPOSITORY  https://github.com/google/snappy.git
  GIT_TAG         1.1.9
)
set(SNAPPY_BUILD_TESTS OFF CACHE BOOL "")
set(SNAPPY_BUILD_BENCHMARKS OFF CACHE BOOL "")
set(SNAPPY_INSTALL OFF CACHE BOOL "")
FetchContent_MakeAvailable(snappy)

add_library(snappy::snappy ALIAS snappy)

# 引入的target：
# snappy::snappy
