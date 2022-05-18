/**
 * @file pb_tools.hpp
 * @author WT
 * @brief protobuf相关工具
 * @note protobuf相关工具
 * @date 2022-05-18
 */
#pragma once

#include <string>

#include <google/protobuf/util/json_util.h>

namespace ytlib {

// 将pb包转换为格式化json字符串
inline std::string Pb2PrettyJson(const google::protobuf::Message& st) {
  google::protobuf::util::JsonPrintOptions op;
  op.always_print_primitive_fields = true;
  op.always_print_enums_as_ints = false;
  op.preserve_proto_field_names = true;
  op.add_whitespace = true;
  std::string str;
  google::protobuf::util::MessageToJsonString(st, &str, op);
  return str;
}

// 将pb包转换为紧凑型json字符串
inline std::string Pb2CompactJson(const google::protobuf::Message& st) {
  google::protobuf::util::JsonPrintOptions op;
  op.always_print_primitive_fields = true;
  op.always_print_enums_as_ints = false;
  op.preserve_proto_field_names = true;
  op.add_whitespace = false;
  std::string str;
  google::protobuf::util::MessageToJsonString(st, &str, op);
  return str;
}
}  // namespace ytlib
