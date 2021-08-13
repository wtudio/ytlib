/**
 * @file string_util.hpp
 * @author WT
 * @brief 基础字符串库
 * @note 常用字符串相关工具
 * @date 2019-07-26
 */
#pragma once

#include <cstring>
#include <map>
#include <set>
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
inline std::map<std::string, std::string> SplitToMap(const std::string& source,
                                                     const std::string& vsep = "&",
                                                     const std::string& msep = "=",
                                                     bool trim = true,
                                                     bool clear = true) {
  std::map<std::string, std::string> result;

  if (source.empty() || vsep.empty() || msep.empty() || vsep == msep) return result;
  size_t v_pos_end, v_pos_start = 0;
  do {
    v_pos_end = source.find(vsep, v_pos_start);
    if (v_pos_end == std::string::npos) v_pos_end = source.length();
    if (v_pos_end > v_pos_start) {
      const std::string& kv_str = source.substr(v_pos_start, v_pos_end - v_pos_start);
      size_t msep_pos = kv_str.find(msep);
      if (msep_pos != std::string::npos) {
        if (trim) {
          std::string first = kv_str.substr(0, msep_pos);
          std::string second = kv_str.substr(msep_pos + msep.size());
          Trim(first);
          if (!(clear && first.empty())) {
            result[first] = Trim(second);
          }
        } else {
          const std::string& first = kv_str.substr(0, msep_pos);
          const std::string& second = kv_str.substr(msep_pos + msep.size());
          if (!(clear && first.empty())) {
            result[first] = second;
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
inline std::string JoinMap(const std::map<std::string, std::string>& kvmap,
                           const std::string& vsep = "&",
                           const std::string& msep = "=") {
  std::string result;
  for (auto itr = kvmap.begin(); itr != kvmap.end(); ++itr) {
    if (itr != kvmap.begin()) result += vsep;
    result += (itr->first + msep + itr->second);
  }
  return result;
}

/**
 * @brief 带默认值的获取map<std::string,std::string>中val数据
 * 
 * @param[in] m std::map<std::string,std::string>结构体
 * @param[in] key 要获取数据的key
 * @param[in] defval 默认字符串，当m中没有对应的key时，返回defval
 * @return std::string 结果
 */
inline std::string GetMapItemWithDef(const std::map<std::string, std::string>& m,
                                     const std::string& key, const std::string& defval = "") {
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
inline std::string& AddKV(std::string& s, const std::string& key, const std::string& val,
                          const std::string& vsep = "&", const std::string& msep = "=") {
  if (!s.empty()) s += vsep;
  s += (key + msep + val);
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
inline std::string GetValueFromStrKV(const std::string& str, const std::string& key,
                                     const std::string& vsep = "&", const std::string& msep = "=",
                                     bool trim = true) {
  if (key.empty() || vsep.empty() || msep.empty() || vsep == msep) return "";
  if (key.find(vsep) != std::string::npos || key.find(msep) != std::string::npos)
    return "";

  size_t key_pos = str.find(key);
  while (key_pos != std::string::npos) {
    size_t msep_pos = str.find(msep, key_pos + key.size());
    if (msep_pos == std::string::npos) return "";

    size_t key_start_pos = str.rfind(vsep, key_pos);
    if (key_start_pos == std::string::npos)
      key_start_pos = 0;
    else
      key_start_pos += vsep.length();

    bool right_key_flag = false;
    if (trim) {
      std::string real_key = str.substr(key_start_pos, msep_pos - key_start_pos);
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
      if (val_end_pos == std::string::npos) val_end_pos = str.length();

      if (trim) {
        std::string re = str.substr(val_start_pos, val_end_pos - val_start_pos);
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
inline std::vector<std::string> SplitToVec(const std::string& source, const std::string& sep,
                                           bool trim = true, bool clear = true) {
  std::vector<std::string> result;
  if (source.empty() || sep.empty()) return result;

  size_t pos_end, pos_start = 0;
  do {
    pos_end = source.find(sep, pos_start);
    if (pos_end == std::string::npos) pos_end = source.length();

    if (trim) {
      std::string sub_str = source.substr(pos_start, pos_end - pos_start);
      Trim(sub_str);
      if (!(clear && sub_str.empty())) {
        result.emplace_back(sub_str);
      }
    } else {
      const std::string& sub_str = source.substr(pos_start, pos_end - pos_start);
      if (!(clear && sub_str.empty())) {
        result.emplace_back(sub_str);
      }
    }
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
inline std::string JoinVec(const std::vector<std::string>& vec, const std::string& sep) {
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
inline std::set<std::string> SplitToSet(const std::string& source, const std::string& sep,
                                        bool trim = true, bool clear = true) {
  std::set<std::string> result;
  if (source.empty() || sep.empty()) return result;

  size_t pos_end, pos_start = 0;
  do {
    pos_end = source.find(sep, pos_start);
    if (pos_end == std::string::npos) pos_end = source.length();

    if (trim) {
      std::string sub_str = source.substr(pos_start, pos_end - pos_start);
      Trim(sub_str);
      if (!(clear && sub_str.empty())) {
        result.emplace(sub_str);
      }
    } else {
      const std::string& sub_str = source.substr(pos_start, pos_end - pos_start);
      if (!(clear && sub_str.empty())) {
        result.emplace(sub_str);
      }
    }
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
inline std::string JoinSet(const std::set<std::string>& st, const std::string& sep) {
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
inline bool CheckIfInList(const std::string& str, const std::string& key,
                          const std::string& sep = ",", bool trim = true) {
  if (key.empty() || sep.empty()) return false;
  if (key.find(sep) != std::string::npos) return false;

  size_t key_pos = str.find(key);
  while (key_pos != std::string::npos) {
    size_t key_start_pos = str.rfind(sep, key_pos);
    if (key_start_pos == std::string::npos)
      key_start_pos = 0;
    else
      key_start_pos += sep.length();

    size_t key_end_pos = str.find(sep, key_pos + key.length());
    if (key_end_pos == std::string::npos) key_end_pos = str.length();

    if (trim) {
      std::string real_key = str.substr(key_start_pos, key_end_pos - key_start_pos);
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
inline int CmpVersion(const std::string& ver1, const std::string& ver2) {
  const std::vector<std::string>& version1_detail = SplitToVec(ver1, ".", true, true);
  const std::vector<std::string>& version2_detail = SplitToVec(ver2, ".", true, true);

  size_t idx = 0;
  for (idx = 0; idx < version1_detail.size() && idx < version2_detail.size(); ++idx) {
    int ver1 = atoi(version1_detail[idx].c_str());
    int ver2 = atoi(version2_detail[idx].c_str());
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
inline bool CheckVersionInside(const std::string& ver, const std::string& start_ver,
                               const std::string& end_ver) {
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
inline std::string& ReplaceString(std::string& str, const std::string& ov, const std::string& nv) {
  if (str.empty() || ov.empty()) return str;
  std::vector<size_t> vec_pos;
  size_t pos = 0, old_len = ov.size(), new_len = nv.size();
  while (std::string::npos != (pos = str.find(ov, pos))) {
    vec_pos.emplace_back(pos);
    pos += old_len;
  }
  size_t& vec_len = pos = vec_pos.size();
  if (vec_len) {
    if (old_len == new_len) {
      for (size_t ii = 0; ii < vec_len; ++ii)
        memcpy(const_cast<char*>(str.c_str() + vec_pos[ii]), nv.c_str(), new_len);
    } else if (old_len > new_len) {
      char* p = const_cast<char*>(str.c_str()) + vec_pos[0];
      vec_pos.emplace_back(str.size());
      for (size_t ii = 0; ii < vec_len; ++ii) {
        memcpy(p, nv.c_str(), new_len);
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
        memcpy(p, nv.c_str(), new_len);
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
inline bool IsAlnumStr(const std::string& str) {
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
inline bool IsDigitStr(const std::string& str) {
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

}  // namespace ytlib
