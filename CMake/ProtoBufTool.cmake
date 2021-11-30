# add protos for target
function(target_add_proto)
  cmake_parse_arguments(ARG "" "TARGET_NAME" "PROTO_PATH;GENCODE_PATH;OPTIONS" ${ARGN})

  # 添加编译前的代码生成
  File(GLOB_RECURSE PROTO_FILES ${ARG_PROTO_PATH}/*.proto)
  foreach(PROTO_FILE ${PROTO_FILES})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" PROTO_FILE_NAME ${PROTO_FILE})
    set(GEN_SRC "${ARG_GENCODE_PATH}/${PROTO_FILE_NAME}.pb.cc")
    set(GEN_HDR "${ARG_GENCODE_PATH}/${PROTO_FILE_NAME}.pb.h")

    add_custom_command(
      OUTPUT ${GEN_SRC} ${GEN_HDR}
      COMMAND protobuf::protoc
      ARGS ${ARG_OPTIONS} --proto_path ${ARG_PROTO_PATH} --cpp_out ${ARG_GENCODE_PATH} ${PROTO_FILE}
      DEPENDS ${PROTO_FILE} protobuf::protoc
      COMMENT "Running cpp protocol buffer compiler on ${PROTO_FILE}. Custom options: ${ARG_OPTIONS}"
      VERBATIM
    )

    target_sources(${ARG_TARGET_NAME} PRIVATE ${GEN_SRC})
    set_property(TARGET ${ARG_TARGET_NAME} PROPERTY PRIVATE_HEADER ${GEN_HDR})

  endforeach()

endfunction()
