include(FetchContent)

message(STATUS "get jsoncpp ...")

FetchContent_Declare(
  jsoncpp
  URL  https://github.com/open-source-parsers/jsoncpp/archive/1.9.5.tar.gz
)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "")
set(JSONCPP_WITH_POST_BUILD_UNITTEST OFF CACHE BOOL "")

FetchContent_MakeAvailable(jsoncpp)

if (TARGET jsoncpp_static)
  add_library(jsoncpp::jsoncpp ALIAS jsoncpp_static)
elseif (TARGET jsoncpp_lib)
  add_library(jsoncpp::jsoncpp ALIAS jsoncpp_lib)
endif ()

# 引入的target：
# jsoncpp::jsoncpp
