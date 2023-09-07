/**
 * @file string_util.hpp
 * @author WT
 * @brief 基础字符串库
 * @note 常用字符串相关工具
 * @date 2019-07-26
 */
#pragma once

#include <algorithm>
#include <cstring>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace ytlib {

/**
 * @brief 修剪字符串
 *
 * @param[inout] s 待处理字符串
 * @return std::string& 处理后的字符串s的引用
 */
inline std::string& Trim(std::string& s) {
  if (s.empty()) return s;
  s.erase(s.find_last_not_of(" ") + 1);
  s.erase(0, s.find_first_not_of(" "));
  return s;
}

inline std::string_view& Trim(std::string_view& s) {
  if (s.empty()) return s;
  auto p = s.find_last_not_of(" ");
  s = s.substr(0, (p != std::string_view::npos) ? (p + 1) : 0);
  p = s.find_first_not_of(" ");
  s = s.substr((p != std::string_view::npos) ? p : s.size());
  return s;
}

/**
 * @brief 将类似于a=1&b=2&c=3这样的字符串解析到map中
 * @note 分割原则：在两个vsep之间的子字符串，如果包含msep，则以msep为界分为key、val
 * @param[in] source 待分割字符串
 * @param[in] vsep 多个kv之间的分隔符，不可为空
 * @param[in] msep 单个kv内部的分隔符，不可为空
 * @param[in] trim 是否去除k和v里的空格
 * @param[in] clear 是否去除key为空的情况
 * @return std::map<std::string, std::string> 解析后的map
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
std::map<StringType, StringType> SplitToMap(std::string_view source,
                                            std::string_view vsep = "&",
                                            std::string_view msep = "=",
                                            bool trim = true,
                                            bool clear = true) {
  std::map<StringType, StringType> result;

  if (source.empty() || vsep.empty() || msep.empty() || vsep == msep) return result;
  size_t v_pos_end, v_pos_start = 0;
  do {
    v_pos_end = source.find(vsep, v_pos_start);
    if (v_pos_end == std::string_view::npos) v_pos_end = source.length();
    if (v_pos_end > v_pos_start) {
      std::string_view kv_str = source.substr(v_pos_start, v_pos_end - v_pos_start);
      size_t msep_pos = kv_str.find(msep);
      if (msep_pos != std::string_view::npos) {
        std::string_view first = kv_str.substr(0, msep_pos);
        std::string_view second = kv_str.substr(msep_pos + msep.size());
        if (trim) {
          Trim(first);
          if (!(clear && first.empty())) {
            result[StringType(first)] = StringType(Trim(second));
          }
        } else {
          if (!(clear && first.empty())) {
            result[StringType(first)] = StringType(second);
          }
        }
      }
    }
    v_pos_start = v_pos_end + vsep.size();
  } while (v_pos_end < source.length());

  return result;
}

/**
 * @brief 拼接map为a=1&b=2&c=3形式的string
 *
 * @param[in] kvmap std::map<std::string,std::string>结构体
 * @param[in] vsep 多个kv之间的分隔符
 * @param[in] msep 单个kv内部的分隔符
 * @return std::string 拼接后的字符串
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
std::string JoinMap(const std::map<StringType, StringType>& kvmap,
                    std::string_view vsep = "&",
                    std::string_view msep = "=") {
  std::stringstream result;
  for (auto itr = kvmap.begin(); itr != kvmap.end(); ++itr) {
    if (itr != kvmap.begin()) result << vsep;
    result << itr->first << msep << itr->second;
  }
  return result.str();
}

/**
 * @brief 带默认值的获取map<std::string,std::string>中val数据
 *
 * @param[in] m std::map<std::string,std::string>结构体
 * @param[in] key 要获取数据的key
 * @param[in] defval 默认字符串，当m中没有对应的key时，返回defval
 * @return std::string 结果
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
const StringType& GetMapItemWithDef(const std::map<StringType, StringType>& m,
                                    const StringType& key, const StringType& defval = "") {
  auto finditr = m.find(key);
  return (finditr != m.end()) ? (finditr->second) : defval;
}

/**
 * @brief 向s中添加kv字段
 *
 * @param[inout] s 待处理字符串
 * @param[in] key key
 * @param[in] val val
 * @param[in] vsep 多个kv之间的分隔符
 * @param[in] msep 单个kv内部的分隔符
 * @return std::string& 拼接后的字符串s的引用
 */
inline std::string& AddKV(std::string& s, std::string_view key, std::string_view val,
                          std::string_view vsep = "&", std::string_view msep = "=") {
  if (!s.empty()) s += vsep;
  s += key;
  s += msep;
  s += val;
  return s;
}

/**
 * @brief 从类似于a=1&b=2&c=3这样的字符串中得到key对应的val
 * @note 比直接分割成map再find要快一些，但如果有多次调用，还是建议先分割成map再find
 * @param[in] str 待处理字符串
 * @param[in] key key，不可为空，不能包含vsep或msep，否则返回空结果
 * @param[in] vsep 多个kv之间的分隔符，不可为空
 * @param[in] msep 单个kv内部的分隔符，不可为空
 * @param[in] trim 是否不计空格
 * @return std::string key对应的val
 */
inline std::string_view GetValueFromStrKV(std::string_view str, std::string_view key,
                                          std::string_view vsep = "&", std::string_view msep = "=",
                                          bool trim = true) {
  if (key.empty() || vsep.empty() || msep.empty() || vsep == msep) return "";
  if (key.find(vsep) != std::string_view::npos || key.find(msep) != std::string_view::npos)
    return "";

  size_t key_pos = str.find(key);
  while (key_pos != std::string_view::npos) {
    size_t msep_pos = str.find(msep, key_pos + key.size());
    if (msep_pos == std::string_view::npos) return "";

    size_t key_start_pos = str.rfind(vsep, key_pos);
    if (key_start_pos == std::string_view::npos)
      key_start_pos = 0;
    else
      key_start_pos += vsep.length();

    bool right_key_flag = false;
    if (trim) {
      std::string_view real_key = str.substr(key_start_pos, msep_pos - key_start_pos);
      Trim(real_key);
      if (real_key == key)
        right_key_flag = true;
    } else {
      if ((key_start_pos == key_pos) && (msep_pos == (key_pos + key.size())))
        right_key_flag = true;
    }

    if (right_key_flag) {
      size_t val_start_pos = msep_pos + msep.size();
      size_t val_end_pos = str.find(vsep, val_start_pos);
      if (val_end_pos == std::string_view::npos) val_end_pos = str.length();

      if (trim) {
        std::string_view re = str.substr(val_start_pos, val_end_pos - val_start_pos);
        return Trim(re);
      } else {
        return str.substr(val_start_pos, val_end_pos - val_start_pos);
      }
    }

    key_pos = str.find(key, msep_pos + msep.length());
  }
  return "";
}

/**
 * @brief 分割字符串到vector
 *
 * @param[in] source 待处理字符串
 * @param[in] sep 分隔符
 * @param[in] trim 是否对每项结果去除空格
 * @param[in] clear 是否去除空项
 * @return std::vector<std::string> 分割结果
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
std::vector<StringType> SplitToVec(std::string_view source, std::string_view sep,
                                   bool trim = true, bool clear = true) {
  std::vector<StringType> result;
  if (source.empty() || sep.empty()) return result;

  size_t pos_end, pos_start = 0;
  do {
    pos_end = source.find(sep, pos_start);
    if (pos_end == std::string_view::npos) pos_end = source.length();

    std::string_view sub_str = source.substr(pos_start, pos_end - pos_start);

    if (trim) Trim(sub_str);

    if (!(clear && sub_str.empty()))
      result.emplace_back(sub_str);

    pos_start = pos_end + sep.size();
  } while (pos_end < source.length());

  return result;
}

/**
 * @brief 拼接vector到string
 *
 * @param[in] vec 待处理vector
 * @param[in] sep 分隔符
 * @return std::string 拼接后的字符串
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
std::string JoinVec(const std::vector<StringType>& vec, std::string_view sep) {
  std::string result;
  for (auto itr = vec.begin(); itr != vec.end(); ++itr) {
    if (itr != vec.begin()) result += sep;
    result += *itr;
  }
  return result;
}

/**
 * @brief 分割字符串到set，自动去重
 *
 * @param[in] source 待处理字符串
 * @param[in] sep 分隔符
 * @param[in] trim 是否对每项结果去除空格
 * @param[in] clear 是否去除空项
 * @return std::set<std::string> 分割结果
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
std::set<StringType> SplitToSet(std::string_view source, std::string_view sep,
                                bool trim = true, bool clear = true) {
  std::set<StringType> result;
  if (source.empty() || sep.empty()) return result;

  size_t pos_end, pos_start = 0;
  do {
    pos_end = source.find(sep, pos_start);
    if (pos_end == std::string_view::npos) pos_end = source.length();

    std::string_view sub_str = source.substr(pos_start, pos_end - pos_start);

    if (trim) Trim(sub_str);

    if (!(clear && sub_str.empty()))
      result.emplace(sub_str);

    pos_start = pos_end + sep.size();
  } while (pos_end < source.length());

  return result;
}

/**
 * @brief 拼接set到string
 *
 * @param[in] st 待处理set
 * @param[in] sep 分隔符
 * @return std::string 拼接后的字符串
 */
template <class StringType = std::string_view>
  requires(
      std::is_same_v<StringType, std::string_view> ||
      std::is_same_v<StringType, std::string>)
std::string JoinSet(const std::set<StringType>& st, std::string_view sep) {
  std::string result;
  for (auto itr = st.begin(); itr != st.end(); ++itr) {
    if (itr != st.begin()) result += sep;
    result += *itr;
  }
  return result;
}

/**
 * @brief 判断key字符串是否在list字符串中，如"123"是否在"123,456,789"中
 * @note 比直接分割成vector再find要快一些，但如果有多次调用，还是建议先分割成vector再find
 * @param[in] str list字符串
 * @param[in] key key字符串，不可为空，不能包含sep
 * @param[in] sep list的分隔符，不可为空
 * @param[in] trim 是否不计空格
 * @return true key在list中
 * @return false key不在list中
 */
inline bool CheckIfInList(std::string_view str, std::string_view key,
                          std::string_view sep = ",", bool trim = true) {
  if (key.empty() || sep.empty()) return false;
  if (key.find(sep) != std::string_view::npos) return false;

  size_t key_pos = str.find(key);
  while (key_pos != std::string_view::npos) {
    size_t key_start_pos = str.rfind(sep, key_pos);
    if (key_start_pos == std::string_view::npos)
      key_start_pos = 0;
    else
      key_start_pos += sep.length();

    size_t key_end_pos = str.find(sep, key_pos + key.length());
    if (key_end_pos == std::string_view::npos) key_end_pos = str.length();

    if (trim) {
      std::string_view real_key = str.substr(key_start_pos, key_end_pos - key_start_pos);
      Trim(real_key);
      if (real_key == key)
        return true;
    } else {
      if ((key_start_pos == key_pos) && (key_end_pos == (key_pos + key.size())))
        return true;
    }

    key_pos = str.find(key, key_end_pos + sep.length());
  }

  return false;
}

/**
 * @brief 比较版本，如6.1.1 6.2.8
 *
 * @param[in] ver1 版本1
 * @param[in] ver2 版本2
 * @return int 返回1是大于，返回0是相等，返回-1是小于
 */
inline int CmpVersion(std::string_view ver1, std::string_view ver2) {
  const auto& version1_detail = SplitToVec<std::string_view>(ver1, ".", true, true);
  const auto& version2_detail = SplitToVec<std::string_view>(ver2, ".", true, true);

  size_t idx = 0;
  for (idx = 0; idx < version1_detail.size() && idx < version2_detail.size(); ++idx) {
    int ver1 = atoi(version1_detail[idx].data());
    int ver2 = atoi(version2_detail[idx].data());
    if (ver1 < ver2)
      return -1;
    else if (ver1 > ver2)
      return 1;
  }
  if (idx == version1_detail.size() && idx == version2_detail.size()) {
    return 0;
  }
  return version1_detail.size() > version2_detail.size() ? 1 : -1;
}

/**
 * @brief 检查版本是否处于设定版本之间
 * @note 需要保证start_ver<end_ver。若end_ver空，则设为999.9.9.9
 * @param[in] ver 待检查版本
 * @param[in] start_ver 开始版本
 * @param[in] end_ver 结束版本
 * @return true 在传入的版本之间
 * @return false 不在传入的版本之间
 */
inline bool CheckVersionInside(std::string_view ver, std::string_view start_ver,
                               std::string_view end_ver) {
  return (CmpVersion(ver, start_ver.empty() ? "0.0.0.0" : start_ver) >= 0 &&
          CmpVersion(ver, end_ver.empty() ? "999.9.9.9" : end_ver) <= 0);
}

/**
 * @brief 将一个字符串中指定字符串ov换为字符串nv
 *
 * @param[inout] str 待替换字符串
 * @param[in] ov 要被替换的子字符串
 * @param[in] nv 要替换成的子字符串
 * @return std::string& 替换后的字符串str的引用
 */
inline std::string& ReplaceString(std::string& str, std::string_view ov, std::string_view nv) {
  if (str.empty() || ov.empty()) return str;
  std::vector<size_t> vec_pos;
  size_t pos = 0, old_len = ov.size(), new_len = nv.size();
  while ((pos = str.find(ov, pos)) != std::string::npos) {
    vec_pos.emplace_back(pos);
    pos += old_len;
  }
  size_t& vec_len = pos = vec_pos.size();
  if (vec_len) {
    if (old_len == new_len) {
      for (size_t ii = 0; ii < vec_len; ++ii)
        memcpy(const_cast<char*>(str.c_str() + vec_pos[ii]), nv.data(), new_len);
    } else if (old_len > new_len) {
      char* p = const_cast<char*>(str.c_str()) + vec_pos[0];
      vec_pos.emplace_back(str.size());
      for (size_t ii = 0; ii < vec_len; ++ii) {
        memcpy(p, nv.data(), new_len);
        p += new_len;
        size_t cplen = vec_pos[ii + 1] - vec_pos[ii] - old_len;
        memmove(p, str.c_str() + vec_pos[ii] + old_len, cplen);
        p += cplen;
      }
      str.resize(p - str.c_str());
    } else {
      size_t diff = new_len - old_len;
      vec_pos.emplace_back(str.size());
      str.resize(str.size() + diff * vec_len);
      char* p = const_cast<char*>(str.c_str()) + str.size();
      for (size_t ii = vec_len - 1; ii < vec_len; --ii) {
        size_t cplen = vec_pos[ii + 1] - vec_pos[ii] - old_len;
        p -= cplen;
        memmove(p, str.c_str() + vec_pos[ii] + old_len, cplen);
        p -= new_len;
        memcpy(p, nv.data(), new_len);
      }
    }
  }
  return str;
}

/**
 * @brief 判断字符串是否为数字和字母组成
 *
 * @param[in] str 待判断字符串
 * @return true 全部为数字和字母组成
 * @return false 不全为数字和字母组成
 */
inline bool IsAlnumStr(std::string_view str) {
  if (str.length() == 0) return false;
  for (auto& c : str) {
    if (!isalnum(c)) return false;
  }
  return true;
}

/**
 * @brief 判断字符串是否为数字组成
 *
 * @param[in] str 待判断字符串
 * @return true 全部为数字组成
 * @return false 不全为数字组成
 */
inline bool IsDigitStr(std::string_view str) {
  if (str.length() == 0) return false;
  for (auto& c : str) {
    if (c > '9' || c < '0') return false;
  }
  return true;
}

/**
 * @brief 获取map中的key的集合
 *
 * @tparam KeyType map中key的类型
 * @tparam ValType map中val的类型
 * @param m 输入map
 * @return std::set<KeyType> key的集合
 */
template <typename KeyType, typename ValType>
std::set<KeyType> GetMapKeys(const std::map<KeyType, ValType>& m) {
  std::set<KeyType> re;
  for (const auto& it : m) re.emplace(it.first);
  return re;
}

/**
 * @brief 字符转小写
 *
 * @param c 字符
 * @return char 小写字符
 */
inline char CharToLower(char c) { return static_cast<char>(tolower(c)); }

/**
 * @brief 字符转大写
 *
 * @param c 字符
 * @return char 大写字符
 */
inline char CharToUpper(char c) { return static_cast<char>(toupper(c)); }

/**
 * @brief 字符串转小写
 *
 * @param str 字符串
 * @return std::string& 小写字符串
 */
inline std::string& StrToLower(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(), CharToLower);
  return str;
}

/**
 * @brief 字符串转大写
 *
 * @param str 字符串
 * @return std::string& 大写字符串
 */
inline std::string& StrToUpper(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(), CharToUpper);
  return str;
}

/**
 * @brief 字符串转为标题形式，单词首字符大写
 *
 * @param str 字符串
 * @return std::string& 标题形式字符串
 */
inline std::string& StrToTitleCase(std::string& str) {
  std::string::iterator it = str.begin();
  *it = CharToUpper(*it);
  for (; it != str.end() - 1; ++it) {
    if (*it == ' ') {
      *(it + 1) = CharToUpper(*(it + 1));
    }
  }
  return str;
}

/**
 * @brief standardise a path string
 *
 * @param path
 * @return std::string&
 */
inline std::string& StandardisePath(std::string& path) {
  if (path.empty()) return (path = "/");
  std::replace(path.begin(), path.end(), '\\', '/');
  if (path[path.length() - 1] != '/') path += '/';
  return path;
}

/**
 * @brief judge if str start with pattern
 *
 * @param str
 * @param pattern
 * @param ignore_case if ignore the case
 * @return true
 * @return false
 */
inline bool StartsWith(std::string_view str, std::string_view pattern, bool ignore_case = false) {
  const size_t str_len = str.length();
  const size_t pattern_len = pattern.length();
  if (str_len < pattern_len || pattern_len == 0) return false;

  if (ignore_case) {
    for (size_t i = 0; i < pattern_len; ++i) {
      if (tolower(pattern[i]) != tolower(str[i])) return false;
    }
    return true;
  } else {
    for (size_t i = 0; i < pattern_len; ++i) {
      if (pattern[i] != str[i]) return false;
    }
    return true;
  }
}

/**
 * @brief judge if str end with pattern
 *
 * @param str
 * @param pattern
 * @param ignore_case if ignore the case
 * @return true
 * @return false
 */
inline bool EndsWith(std::string_view str, std::string_view pattern, bool ignore_case = false) {
  const size_t str_len = str.length();
  const size_t pattern_len = pattern.length();
  if (str_len < pattern_len || pattern_len == 0) return false;

  const size_t& begin_pos = str_len - pattern_len;
  if (ignore_case) {
    for (size_t i = 0; i < pattern_len; ++i) {
      if (tolower(pattern[i]) != tolower(str[begin_pos + i])) return false;
    }
    return true;
  } else {
    for (size_t i = 0; i < pattern_len; i++) {
      if (pattern[i] != str[begin_pos + i]) return false;
    }
    return true;
  }
}

/**
 * @brief fnv1a hash算法，64位
 *
 * @param data
 * @param len
 * @return const uint64_t hash值
 */
inline const uint64_t Hash64Fnv1a(const char* data, size_t len) {
  const uint64_t prime = 0x100000001b3;
  uint64_t hash = 0xcbf29ce484222325;
  for (size_t i = 0; i < len; ++i) {
    hash = (hash ^ static_cast<uint8_t>(data[i])) * prime;
  }
  return hash;
}

/**
 * @brief fnv1a hash算法，32位
 *
 * @param data
 * @param len
 * @return const uint32_t hash值
 */
inline const uint32_t Hash32Fnv1a(const char* data, size_t len) {
  const uint32_t prime = 0x1000193;
  uint32_t hash = 0x811c9dc5;
  for (size_t i = 0; i < len; ++i) {
    hash = (hash ^ static_cast<uint8_t>(data[i])) * prime;
  }
  return hash;
}

}  // namespace ytlib
