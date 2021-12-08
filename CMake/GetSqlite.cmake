include(FetchContent)

message(STATUS "get sqlite ...")

# sqlite版本记录：https://www.sqlite.org/chronology.html
FetchContent_Declare(
  sqlite
  URL  https://sqlite.org/2021/sqlite-amalgamation-3370000.zip
)

FetchContent_GetProperties(sqlite)
if(NOT sqlite_POPULATED)
  FetchContent_Populate(sqlite)
endif()

# sqlite exe
add_library(sqlite)
add_library(sqlite::sqlite ALIAS sqlite)

target_sources(sqlite PRIVATE ${sqlite_SOURCE_DIR}/sqlite3.c ${sqlite_SOURCE_DIR}/shell.c)
target_include_directories(sqlite PRIVATE ${sqlite_SOURCE_DIR})

# sqlite lib
add_library(libsqlite)
add_library(sqlite::libsqlite ALIAS libsqlite)

target_sources(libsqlite PRIVATE ${sqlite_SOURCE_DIR}/sqlite3.c)
target_include_directories(libsqlite PUBLIC ${sqlite_SOURCE_DIR})

# 引入的target：
# sqlite::sqlite
# sqlite::libsqlite