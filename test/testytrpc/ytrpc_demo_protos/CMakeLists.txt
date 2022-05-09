add_protobuf_gencode_for_path_target(
  TARGET_NAME ytrpc_demo_protos_gencode
  PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
add_library(ytlib::ytrpc_demo_protos_gencode ALIAS ytrpc_demo_protos_gencode)

add_ytrpc_gencode_for_one_file_target(
  TARGET_NAME ytrpc_demo_rpc_gencode
  PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Demo.proto
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
target_link_libraries(ytrpc_demo_rpc_gencode INTERFACE ytlib::ytrpc ytlib::ytrpc_demo_protos_gencode)
add_library(ytlib::ytrpc_demo_rpc_gencode ALIAS ytrpc_demo_rpc_gencode)
