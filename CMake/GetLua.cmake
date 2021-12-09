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

# lua exe
add_executable(lua)

File(GLOB lua_src ${lua_SOURCE_DIR}/*.c)
LIST(REMOVE_ITEM lua_src ${lua_SOURCE_DIR}/onelua.c)

target_sources(lua PRIVATE ${lua_src})
target_include_directories(lua PRIVATE ${lua_SOURCE_DIR})

# lua lib
add_library(liblua)
add_library(lua::liblua ALIAS liblua)

File(GLOB lib_lua_src ${lua_SOURCE_DIR}/*.c)
LIST(REMOVE_ITEM lib_lua_src ${lua_SOURCE_DIR}/lua.c ${lua_SOURCE_DIR}/onelua.c)

target_sources(liblua PRIVATE ${lib_lua_src})
target_include_directories(liblua PUBLIC ${lua_SOURCE_DIR})

# 引入的target：
# lua::lua
# lua::liblua
