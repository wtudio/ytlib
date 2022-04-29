add_protobuf_gencode_for_path_target(
  TARGET_NAME testgencode
  PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
add_library(testgencode::testgencode ALIAS testgencode)

add_testrpc_gencode_for_path_target(
  TARGET_NAME testgencode_rpc
  PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR}
  GENCODE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gen
)
target_link_libraries(testgencode_rpc INTERFACE testgencode::testgencode)
add_library(testgencode::testgencode_rpc ALIAS testgencode_rpc)
