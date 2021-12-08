#pragma once

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <map>
#include <string>
#include <vector>

namespace ytlib {

// 常用json类型
enum class JSON_NODE_TYPE : uint8_t {
  OBJ,
  STR,
  ARR,
  INT,
  DBL,
  NUM,
  BOOL,
  NUL,
};

// 判断一个节点是否是object并且有对应类型的child
inline bool CheckChild(const rapidjson::Value& val, const char* child,
                       JSON_NODE_TYPE type = JSON_NODE_TYPE::OBJ) {
  if (!val.IsObject()) return false;
  if (!val.HasMember(child)) return false;
  switch (type) {
    case JSON_NODE_TYPE::OBJ:
      return val[child].IsObject();
    case JSON_NODE_TYPE::STR:
      return val[child].IsString();
    case JSON_NODE_TYPE::ARR:
      return val[child].IsArray();
    case JSON_NODE_TYPE::INT:
      return val[child].IsInt();
    case JSON_NODE_TYPE::DBL:
      return val[child].IsDouble();
    case JSON_NODE_TYPE::NUM:
      return val[child].IsNumber();
    case JSON_NODE_TYPE::BOOL:
      return val[child].IsBool();
    case JSON_NODE_TYPE::NUL:
      return val[child].IsNull();
    default:
      return false;
  }
}

// 批量判断是否有对应的子节点
inline bool CheckChild(const rapidjson::Value& val,
                       const std::map<std::string, JSON_NODE_TYPE>& m) {
  for (auto& item : m) {
    if (!CheckChild(val, item.first.c_str(), item.second)) return false;
  }
  return true;
}

/**
 * @brief 找符合要求的子节点
 * @param[in] p_root 根节点
 * @param[inout] p_vec 满足要求的节点列表
 * @param[in] f 判断节点是否满足要求的判断函数
 * @return 无
 */
inline void FindVal(rapidjson::Value* p_root, std::vector<rapidjson::Value*>* p_vec,
                    std::function<bool(const rapidjson::Value&)> f) {
  // 如果自身就符合，则不去再找子节点
  if (f(*p_root)) {
    p_vec->push_back(p_root);
    return;
  }

  if (p_root->IsObject()) {
    // 如果自身是一个obj，则遍历所有child
    for (auto& m : p_root->GetObject()) FindVal(&(m.value), p_vec, f);
  } else if (p_root->IsArray()) {
    // 如果自身是一个array，则遍历所有array值
    for (auto& v : p_root->GetArray()) FindVal(&v, p_vec, f);
  }
  return;
}

/**
 * @brief 将map<string,string>添加到指定json节点下
 * @param[inout] p_val 待添加map的节点，必须为Object类型，否则不会添加
 * @param[in] m 要添加的map
 * @param[in] jalc rapidjson的内存分配器
 * @return 无
 */
inline void AddMap(rapidjson::Value* p_val, const std::map<std::string, std::string>& m,
                   rapidjson::Document::AllocatorType& jalc) {
  if (!(p_val->IsObject())) return;
  for (auto& itr : m)
    p_val->AddMember(rapidjson::Value().SetString(itr.first.c_str(), jalc),
                     rapidjson::Value().SetString(itr.second.c_str(), jalc), jalc);
}

// json结构体转格式化字符串
inline std::string JsonObj2PrettyStr(const rapidjson::Value* p_val) {
  rapidjson::StringBuffer buffer;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
  p_val->Accept(writer);
  return buffer.GetString();
}

// json结构体转紧凑型字符串
inline std::string JsonObj2CompactStr(const rapidjson::Value* p_val) {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  p_val->Accept(writer);
  return buffer.GetString();
}

// 检查是否有对应类型子节点
inline bool CheckChildObj(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::OBJ);
}
inline bool CheckChildStr(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::STR);
}
inline bool CheckChildArr(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::ARR);
}
inline bool CheckChildInt(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::INT);
}
inline bool CheckChildDbl(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::DBL);
}
inline bool CheckChildNum(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::NUM);
}
inline bool CheckChildBool(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::BOOL);
}
inline bool CheckChildNul(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::NUL);
}

// 如果有对应类型子节点，则获取其值
inline const char* GetChildStr(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::STR) ? (val[child].GetString()) : "";
}
inline int GetChildInt(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::INT) ? (val[child].GetInt()) : 0;
}
inline double GetChildDbl(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::DBL) ? (val[child].GetDouble()) : 0.0;
}
inline bool GetChildBool(const rapidjson::Value& val, const char* child) {
  return CheckChild(val, child, JSON_NODE_TYPE::BOOL) ? (val[child].GetBool()) : false;
}

}  // namespace ytlib