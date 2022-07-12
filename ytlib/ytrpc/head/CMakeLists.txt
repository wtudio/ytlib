add_protobuf_gencode_for_one_file_target(
  TARGET_NAME head_proto_gencode
  PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Head.proto
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
add_library(ytlib::ytrpc::head_proto_gencode ALIAS head_proto_gencode)
