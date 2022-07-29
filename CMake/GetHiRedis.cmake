include(FetchContent)

message(STATUS "get hiredis ...")

FetchContent_Declare(
  hiredis
  URL  https://github.com/redis/hiredis/archive/eaa2a7ee77f4ce25e73a23e6030d4fa4d138cb11.tar.gz
)

set(DISABLE_TESTS ON CACHE BOOL "")

FetchContent_MakeAvailable(hiredis)

if(NOT TARGET hiredis::hiredis_auto)
  if(BUILD_SHARED_LIBS)
    add_library(hiredis::hiredis_auto ALIAS hiredis)
  else()
    add_library(hiredis::hiredis_auto ALIAS hiredis_static)
  endif()
endif()

# import targets：
# hiredis::hiredis
# hiredis::hiredis_static
# hiredis::hiredis_auto
