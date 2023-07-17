# Get the current folder name
string(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Set target name
set(CUR_TARGET_NAME ${CUR_DIR})

# Set file collection
file(GLOB_RECURSE head_files ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/*_test.cpp)
file(GLOB_RECURSE benchmark_files ${CMAKE_CURRENT_SOURCE_DIR}/*_benchmark.cpp)

# Add target
add_library(${CUR_TARGET_NAME} INTERFACE)
add_library(ytlib::${CUR_TARGET_NAME} ALIAS ${CUR_TARGET_NAME})

# Set include path of target
target_include_directories(
  ${CUR_TARGET_NAME}
  INTERFACE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)

# Set head files of target
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY PUBLIC_HEADER ${head_files})

# Set link libraries of target
# target_link_libraries(${CUR_TARGET_NAME} INTERFACE xxx)

# Set compile definitions of target
# target_compile_definitions(${CUR_TARGET_NAME} INTERFACE xxx)

# Set installation of target
install(TARGETS ${CUR_TARGET_NAME} PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ytlib/${CUR_DIR})

# Set test of target
if(YTLIB_BUILD_TESTS AND test_files)
  add_gtest_target(TEST_TARGET ${CUR_TARGET_NAME} TEST_SRC ${test_files})
endif()

if(YTLIB_BUILD_BENCH_TESTS AND benchmark_files)
  add_benchmark_target(BENCH_TARGET ${CUR_TARGET_NAME} BENCH_SRC ${benchmark_files})
endif()
