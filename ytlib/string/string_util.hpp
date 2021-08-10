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
 * 
 * @param[in] source 待分割字符串
 * @param[in] vsep 多个kv之间的分隔符
 * @param[in] msep 单个kv内部的分隔符
 * @param[in] trimempty 是否去除k和v里的空格
 * @return std::map<std::string, std::string> 解析后的map
 */
inline std::map<std::string, std::string> SplitToMap(const std::string& source,
                                                     const std::string& vsep = "&",
                                                     const std::string& msep = "=",
                                                     bool trimempty = true) {
  std::map<std::string, std::string> result;

  const std::string& str = source + vsep;

  size_t n, pos = 0;
  while ((n = str.find(vsep, pos)) != std::string::npos) {
    const std::string& sub = str.substr(pos, n - pos);
    if (!sub.empty()) {
      size_t m;
      if ((m = sub.find(msep)) != std::string::npos) {
        if (trimempty) {
          std::string first = sub.substr(0, m);
          std::string second = sub.substr(m + msep.size());
          result[Trim(first)] = Trim(second);
        } else {
          result[sub.substr(0, m)] = sub.substr(m + msep.size());
        }
      }
    }
    pos = n + vsep.size();
  }

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
  for (auto& itr : kvmap) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += vsep;
    result += itr.first + msep + itr.second;
  }
  return result;
}

/**
 * @brief 带默认值的获取map<std::string,std::string>中val数据
 * 
 * @param[in] m std::map<std::string,std::string>结构体
 * @param[in] key 要获取数据的key
 * @param[in] defval 默认字符串，当m中没有对应的key时，返回defval
 * @return const std::string& 结果的const引用
 */
inline const std::string& GetMapItemWithDef(const std::map<std::string, std::string>& m,
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
 * @param[in] key key，不能包含vsep或msep，否则返回空结果
 * @param[in] vsep 多个kv之间的分隔符
 * @param[in] msep 单个kv内部的分隔符
 * @param[in] trimempty 是否不计空格
 * @return std::string key对应的val
 */
inline std::string GetValueFromStrKV(const std::string& str, const std::string& key,
                                     const std::string& vsep = "&", const std::string& msep = "=",
                                     bool trimempty = true) {
  if (key.find(vsep) != std::string::npos || key.find(msep) != std::string::npos)
    return "";

  size_t pos = str.find(key);
  if (std::string::npos == pos) return "";

  pos = str.find(msep, pos);
  if (std::string::npos == pos) return "";
  pos += msep.length();

  size_t pos_end = str.find(vsep, pos);
  if (std::string::npos == pos_end) pos_end = str.length();

  std::string re = str.substr(pos, pos_end - pos);
  if (trimempty) Trim(re);

  return re;
}

/**
 * @brief 分割字符串到vector
 * 
 * @param[in] source 待处理字符串。连续分隔符视为1个
 * @param[in] sep 分隔符
 * @param[in] trimempty 是否对每项结果去除空格。如果为true则等效为空格也是分隔符。默认true
 * @return std::vector<std::string> 分割结果
 */
inline std::vector<std::string> SplitToVec(const std::string& source, const std::string& sep,
                                           bool trimempty = true) {
  std::vector<std::string> re;
  size_t pos1, pos2 = 0;
  const std::string& real_sep = trimempty ? (sep + " ") : sep;
  do {
    pos1 = source.find_first_not_of(real_sep, pos2);
    if (pos1 == std::string::npos) break;
    pos2 = source.find_first_of(real_sep, pos1);
    re.emplace_back(source.substr(pos1, pos2 - pos1));
  } while (pos2 != std::string::npos);
  return re;
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
 * @param[in] trimempty 是否对每项结果去除空格
 * @return std::set<std::string> 分割结果
 */
inline std::set<std::string> SplitToSet(const std::string& source, const std::string& sep,
                                        bool trimempty = true) {
  std::set<std::string> re;
  size_t pos1, pos2 = 0;
  const std::string& real_sep = trimempty ? (sep + " ") : sep;
  do {
    pos1 = source.find_first_not_of(real_sep, pos2);
    if (pos1 == std::string::npos) break;
    pos2 = source.find_first_of(real_sep, pos1);
    re.emplace(source.substr(pos1, pos2 - pos1));
  } while (pos2 != std::string::npos);
  return re;
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
 * @note 注意，key中不可包含分隔符，key、sep不可为空，否则返回false
 * @param[in] strlist list字符串
 * @param[in] key key字符串
 * @param[in] sep list的分隔符
 * @param[in] trimempty 是否对每项结果去除空格
 * @return true key在list中
 * @return false key不在list中
 */
inline bool CheckIfInList(const std::string& strlist, const std::string& key,
                          const std::string& sep = ",", bool trimempty = true) {
  if (key.empty() || sep.empty()) return false;

  const std::string& real_sep = trimempty ? (sep + " ") : sep;
  if (key.find_first_of(real_sep) != std::string::npos) return false;

  size_t pos = strlist.find(key);
  while (pos != std::string::npos) {
    if ((pos > 0 && real_sep.find(strlist[pos - 1]) == std::string::npos) ||
        ((pos + key.length()) < strlist.length() && real_sep.find(strlist[pos + key.length()]) == std::string::npos)) {
      // 如果前后有不是分割符的情况就查下一个
      pos = strlist.find(key, pos + key.length());
    } else {
      return true;
    }
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
  const std::vector<std::string>& version1_detail = SplitToVec(ver1, ".");
  const std::vector<std::string>& version2_detail = SplitToVec(ver2, ".");

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
