
add_subdirectory(protoc-gen-testrpc)

function(add_testrpc_gencode_for_path_target)
  cmake_parse_arguments(ARG "" "TARGET_NAME" "PROTO_PATH;GENCODE_PATH;OPTIONS" ${ARGN})

  if(NOT EXISTS ${ARG_GENCODE_PATH})
    file(MAKE_DIRECTORY ${ARG_GENCODE_PATH})
  endif()

  set(GEN_SRCS)
  set(GEN_HDRS)

  File(GLOB_RECURSE PROTO_FILES ${ARG_PROTO_PATH}/*.proto)
  foreach(PROTO_FILE ${PROTO_FILES})
    STRING(REGEX REPLACE ".+/(.+)\\..*" "\\1" PROTO_FILE_NAME ${PROTO_FILE})
    set(GEN_SRC "${ARG_GENCODE_PATH}/${PROTO_FILE_NAME}.testrpc.pb.cc")
    set(GEN_HDR "${ARG_GENCODE_PATH}/${PROTO_FILE_NAME}.testrpc.pb.h")

    list(APPEND GEN_SRCS ${GEN_SRC})
    list(APPEND GEN_HDRS ${GEN_HDR})

    add_custom_command(
      OUTPUT ${GEN_SRC} ${GEN_HDR}
      COMMAND protobuf::protoc
      ARGS ${ARG_OPTIONS} --proto_path ${ARG_PROTO_PATH} --testrpc_out ${ARG_GENCODE_PATH} --plugin=protoc-gen-testrpc=$<TARGET_FILE:ytlib::protoc-gen-testrpc> ${PROTO_FILE}
      DEPENDS ${PROTO_FILE} protobuf::protoc ytlib::protoc-gen-testrpc
      COMMENT "Running protoc, args: ${ARG_OPTIONS} --proto_path ${ARG_PROTO_PATH} --testrpc_out ${ARG_GENCODE_PATH} --plugin=protoc-gen-testrpc=$<TARGET_FILE:ytlib::protoc-gen-testrpc> ${PROTO_FILE}"
      VERBATIM
    )
  endforeach()

  add_library(${ARG_TARGET_NAME} INTERFACE)

  target_sources(${ARG_TARGET_NAME} PUBLIC ${GEN_SRCS})
  target_include_directories(${ARG_TARGET_NAME} INTERFACE ${ARG_GENCODE_PATH})
  set_property(TARGET ${ARG_TARGET_NAME} PROPERTY PUBLIC_HEADER ${GEN_HDRS})

endfunction()

add_subdirectory(protos)

add_subdirectory(rpccli)
add_subdirectory(rpcsvr)