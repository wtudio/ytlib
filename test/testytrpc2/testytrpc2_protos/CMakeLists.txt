add_protobuf_gencode_for_path_target(
  TARGET_NAME testytrpc2_protos_gencode
  PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
add_library(ytlib::testytrpc2_protos_gencode ALIAS testytrpc2_protos_gencode)

add_ytrpc_gencode_for_one_file_target(
  TARGET_NAME testytrpc2_rpc_gencode
  PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.proto
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
target_link_libraries(testytrpc2_rpc_gencode INTERFACE ytlib::ytrpc ytlib::testytrpc2_protos_gencode)
add_library(ytlib::testytrpc2_rpc_gencode ALIAS testytrpc2_rpc_gencode)
