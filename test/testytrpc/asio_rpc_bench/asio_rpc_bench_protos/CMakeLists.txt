add_protobuf_gencode_for_path_target(TARGET_NAME asio_rpc_bench_protos_gencode PROTO_PATH ${CMAKE_CURRENT_SOURCE_DIR} GENCODE_PATH
                                     ${CMAKE_CURRENT_SOURCE_DIR}/gen)
add_library(testytrpc::asio_rpc_bench_protos_gencode ALIAS asio_rpc_bench_protos_gencode)

add_asio_rpc_gencode_for_one_file_target(TARGET_NAME asio_rpc_bench_rpc_gencode PROTO_FILE ${CMAKE_CURRENT_SOURCE_DIR}/helloworld.proto GENCODE_PATH
                                         ${CMAKE_CURRENT_SOURCE_DIR}/gen)
add_library(testytrpc::asio_rpc_bench_rpc_gencode ALIAS asio_rpc_bench_rpc_gencode)
