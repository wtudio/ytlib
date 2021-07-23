#pragma once

#include <cstring>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace ytlib {

/**
 * @brief 修剪字符串
 * @param s 待处理字符串
 * @return 处理后的字符串
 */
inline std::string& trim(std::string& s) {
  if (s.empty()) return s;
  s.erase(s.find_last_not_of(" ") + 1);
  s.erase(0, s.find_first_not_of(" "));
  return s;
}

/**
 * @brief 将类似于a=1&b=2&c=3这样的字符串解析到map中
 * @param source 待分割字符串
 * @param vsep 多个kv之间的分隔符
 * @param msep 单个kv内部的分隔符
 * @param trimempty 是否去除空格
 * @return 解析后的map
 */
inline std::map<std::string, std::string> SplitToMap(const std::string& source,
                                                     const std::string& vsep = "&",
                                                     const std::string& msep = "=",
                                                     bool trimempty = true) {
  std::map<std::string, std::string> result;
  if (source.empty()) return result;

  size_t m, n, pos = 0;
  std::string str = source;
  str.append(vsep);

  std::string sub;
  std::string first;
  std::string second;
  while ((n = str.find(vsep, pos)) != std::string::npos) {
    sub = str.substr(pos, n - pos);
    if (!sub.empty()) {
      if ((m = sub.find(msep)) != std::string::npos) {
        first = sub.substr(0, m);
        second = sub.substr(m + msep.size());
        if (trimempty) {
          result[trim(first)] = trim(second);
        } else {
          result[first] = second;
        }
      }
    }
    pos = n + vsep.size();
  }

  return result;
}

/**
 * @brief 拼接map为a=1&b=2&c=3形式的string
 * @param kvmap std::map<std::string,std::string>结构体
 * @param vsep 多个kv之间的分隔符
 * @param msep 单个kv内部的分隔符
 * @return 拼接后的字符串
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
 * @param m std::map<std::string,std::string>结构体
 * @param key 要获取数据的key
 * @param defval 默认字符串
 * @return 当m中没有对应的key时，返回defval
 */
inline std::string GetMapItemWithDef(const std::map<std::string, std::string>& m,
                                     const std::string& key, const std::string& defval = "") {
  auto finditr = m.find(key);
  return (finditr != m.end()) ? (finditr->second) : defval;
}

/**
 * @brief 向s中添加kv字段
 * @param s 待处理字符串
 * @param key key
 * @param val val
 * @param vsep 多个kv之间的分隔符
 * @param msep 单个kv内部的分隔符
 * @return 拼接后的字符串
 */
inline std::string& AddKV(std::string& s, const std::string& key, const std::string& val,
                          const std::string& vsep = "&", const std::string& msep = "=") {
  if (!s.empty()) s += vsep;
  s += (key + msep + val);
  return s;
}

/**
 * @brief 从类似于a=1&b=2&c=3这样的字符串中得到key对应的val
 * @param str 待处理字符串
 * @param key key
 * @param vsep 多个kv之间的分隔符
 * @param msep 单个kv内部的分隔符
 * @param trimempty 是否对每项结果去除空格
 * @return key对应的val
 */
inline std::string GetValueFromStrKV(const std::string& str, const std::string& key,
                                     const std::string& vsep = "&", const std::string& msep = "=",
                                     bool trimempty = true) {
  size_t pos = str.find(key);
  if (std::string::npos == pos) return "";

  pos = str.find(msep, pos);
  if (std::string::npos == pos) return "";
  pos += msep.length();

  size_t pos_end = str.find(vsep, pos);
  if (std::string::npos == pos_end) pos_end = str.length();

  std::string re = str.substr(pos, pos_end - pos);
  if (trimempty) trim(re);

  return re;
}

/**
 * @brief 分割字符串到vector
 * @param source 待处理字符串
 * @param sep 分隔符
 * @param trimempty 是否对每项结果去除空格
 * @return 分割结果
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
 * @param vec 待处理vector
 * @param sep 分隔符
 * @return 拼接后的字符串
 */
inline std::string JoinVec(const std::vector<std::string>& vec, const std::string& sep) {
  std::string result;
  for (auto& itr : vec) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += sep;
    result += itr;
  }
  return result;
}

/**
 * @brief 分割字符串到set，自动去重
 * @param source 待处理字符串
 * @param sep 分隔符
 * @param trimempty 是否对每项结果去除空格
 * @return 分割结果
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
 * @param st 待处理set
 * @param sep 分隔符
 * @return 拼接后的字符串
 */
inline std::string JoinSet(const std::set<std::string>& st, const std::string& sep) {
  std::string result;
  for (auto& itr : st) {
    // 若不为空，则需要增加分隔符
    if (!result.empty()) result += sep;
    result += itr;
  }
  return result;
}

/**
 * @brief 判断key字符串是否在list字符串中，如"123"是否在"123,456,789"中
 * 注意，key中不可包含分隔符，key不可为空，否则返回false
 * @param strlist list字符串
 * @param key key字符串
 * @param sep list的分隔符
 * @return key是否在list中
 */
inline bool CheckIfInList(const std::string& strlist, const std::string& key, char sep = ',') {
  if (key.empty()) return false;
  if (key.find(sep) != std::string::npos) return false;
  size_t pos = strlist.find(key);
  while (pos != std::string::npos) {
    if ((pos > 0 && strlist[pos - 1] != sep) ||
        ((pos + key.length()) < strlist.length() && strlist[pos + key.length()] != sep)) {
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
 * @param ver1 版本1
 * @param ver2 版本2
 * @return 返回1是大于，返回0是相等，返回-1是小于
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
 * @brief 检查版本是否处于设定版本之间。需要保证start_ver<end_ver。若end_ver空，则设为999.9.9.9
 * @param ver 版本
 * @param start_ver 开始版本
 * @param end_ver 结束版本
 * @return 返回true是在传入的版本之间
 */
inline bool CheckVersionInside(const std::string& ver, const std::string& start_ver,
                               const std::string& end_ver) {
  return (CmpVersion(ver, start_ver.empty() ? "0.0.0.0" : start_ver) >= 0 &&
          CmpVersion(ver, end_ver.empty() ? "999.9.9.9" : end_ver) <= 0);
}

/**
 * @brief 将一个字符串中指定字符串ov换为字符串nv
 * @param str 待替换字符串
 * @param ov 要被替换的子字符串
 * @param nv 要替换成的子字符串
 * @return 替换后的字符串
 */
inline std::string& ReplaceString(std::string& str, const std::string& ov, const std::string& nv) {
  if (str.empty()) return str;
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
 * @param str 待判断字符串
 * @return 是否为数字和字母组成
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
 * @param str 待判断字符串
 * @return 是否为数字组成
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
 * @param ValType map中val的类型
 * @param m 输入map
 * @return key的集合
 */
template <typename KeyType, typename ValType>
std::set<KeyType> GetMapKeys(const std::map<KeyType, ValType>& m) {
  std::set<KeyType> re;
  for (const auto& it : m) re.emplace(it.first);
  return re;
}

}  // namespace ytlib
