# add target for unifex rpc gen code for one file
function(add_unifex_rpc_gencode_for_one_file_target)
  cmake_parse_arguments(ARG "" "TARGET_NAME" "PROTO_FILE;GENCODE_PATH;OPTIONS" ${ARGN})

  if(NOT EXISTS ${ARG_GENCODE_PATH})
    file(MAKE_DIRECTORY ${ARG_GENCODE_PATH})
  endif()

  string(REGEX REPLACE ".+/(.+)\\..*" "\\1" PROTO_FILE_NAME ${ARG_PROTO_FILE})
  string(REGEX REPLACE "(.+)/(.+)\\..*" "\\1" PROTO_PATH ${ARG_PROTO_FILE})
  set(GEN_SRC "${ARG_GENCODE_PATH}/${PROTO_FILE_NAME}.unifex_rpc.pb.cc")
  set(GEN_HDR "${ARG_GENCODE_PATH}/${PROTO_FILE_NAME}.unifex_rpc.pb.h")

  add_custom_command(
    OUTPUT ${GEN_SRC} ${GEN_HDR}
    COMMAND protobuf::protoc ARGS ${ARG_OPTIONS} --proto_path ${PROTO_PATH} --unifex_rpc_out ${ARG_GENCODE_PATH}
            --plugin=protoc-gen-unifex_rpc=$<TARGET_FILE:ytlib::ytrpc::protoc-gen-unifex_rpc> ${ARG_PROTO_FILE}
    DEPENDS ${ARG_PROTO_FILE} protobuf::protoc ytlib::ytrpc::protoc-gen-unifex_rpc
    COMMENT
      "Running protoc, args: ${ARG_OPTIONS} --proto_path ${PROTO_PATH} --unifex_rpc_out ${ARG_GENCODE_PATH} --plugin=protoc-gen-unifex_rpc=$<TARGET_FILE:ytlib::ytrpc::protoc-gen-unifex_rpc> ${ARG_PROTO_FILE}"
    VERBATIM)

  add_library(${ARG_TARGET_NAME} INTERFACE)

  target_sources(${ARG_TARGET_NAME} PUBLIC ${GEN_SRC})
  target_include_directories(${ARG_TARGET_NAME} INTERFACE ${ARG_GENCODE_PATH})
  set_property(TARGET ${ARG_TARGET_NAME} PROPERTY PUBLIC_HEADER ${GEN_HDR})

endfunction()
