include(FetchContent)

message(STATUS "get lua ...")

FetchContent_Declare(
  lua
  URL  https://github.com/lua/lua/archive/v5.4.3.tar.gz
)

FetchContent_GetProperties(lua)
if(NOT lua_POPULATED)
  FetchContent_Populate(lua)
endif()

File(GLOB src ${lua_SOURCE_DIR}/*.c)
File(GLOB lib_src ${src})
LIST(REMOVE_ITEM lib_src ${lua_SOURCE_DIR}/lua.c ${lua_SOURCE_DIR}/onelua.c)

# lua exe
add_library(lua)
add_library(lua::lua ALIAS lua)

target_sources(lua PRIVATE ${src})
target_include_directories(lua PUBLIC ${lua_SOURCE_DIR})

# lua lib
add_library(liblua)
add_library(lua::liblua ALIAS liblua)

target_sources(liblua PRIVATE ${lib_src})
target_include_directories(liblua PUBLIC ${lua_SOURCE_DIR})

# 引入的target：
# lua::lua
# lua::liblua
