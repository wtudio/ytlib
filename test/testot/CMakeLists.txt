# Get the current folder name
string(REGEX REPLACE ".*/\(.*\)" "\\1" CUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Set target name
set(CUR_TARGET_NAME ${CUR_DIR})

# Set file collection
file(GLOB_RECURSE head_files ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

# Add target
add_executable(${CUR_TARGET_NAME})

# Set source file of target
target_sources(${CUR_TARGET_NAME} PRIVATE ${src})

# Set include path of target
target_include_directories(
  ${CUR_TARGET_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Set head files of target
set_property(TARGET ${CUR_TARGET_NAME} PROPERTY PRIVATE_HEADER ${head_files})

# Set link libraries of target
target_link_libraries(
  ${CUR_TARGET_NAME}
  PRIVATE ytlib::misc
          opentelemetry-cpp::api
          opentelemetry-cpp::sdk
          opentelemetry-cpp::ext
          opentelemetry-cpp::version
          opentelemetry-cpp::common
          opentelemetry-cpp::trace
          opentelemetry-cpp::metrics
          opentelemetry-cpp::logs
          opentelemetry-cpp::ostream_span_exporter
          opentelemetry-cpp::ostream_metrics_exporter
          opentelemetry-cpp::ostream_log_record_exporter
          opentelemetry-cpp::otlp_recordable
          opentelemetry-cpp::otlp_http_client
          opentelemetry-cpp::otlp_http_exporter
          opentelemetry-cpp::otlp_http_log_record_exporter
          opentelemetry-cpp::otlp_http_metric_exporter
          opentelemetry-cpp::http_client_curl)

# Set compile definitions of target
# target_compile_definitions(${CUR_TARGET_NAME} PRIVATE xxx)
