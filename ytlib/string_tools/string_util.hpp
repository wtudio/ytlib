#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "string_tools_exports.h"

namespace ytlib {

/**
 * @brief 带默认值的获取数据
 * @param val 数据字符串
 * @param defval 默认字符串
 * @return 当val参数为空时，返回defval
 */
inline std::string WithDef(const std::string& val, const std::string& defval = "") {
  return val.empty() ? defval : val;
}

/**
 * @brief 修剪字符串
 * @param s 待处理字符串
 * @return 处理后的字符串
 */
STRING_TOOLS_API std::string& trim(std::string& s);

/**
 * @brief 带默认值的获取map<string,string>中val数据
 * @param m map<string,string>结构体
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
 * @brief 将类似于a=1&b=2&c=3这样的字符串解析到map中
 * @param source 待分割字符串
 * @param vsep 多个kv之间的分隔符
 * @param msep 单个kv内部的分隔符
 * @param trimempty 是否去除空格
 * @return 解析后的map
 */
STRING_TOOLS_API std::map<std::string, std::string> SplitToMap(const std::string& source,
                                                               const std::string& vsep = "&",
                                                               const std::string& msep = "=", bool trimempty = true);

/**
 * @brief 拼接map为a=1&b=2&c=3形式的string
 * @param kvmap map<string,string>结构体
 * @param vsep 多个kv之间的分隔符
 * @param msep 单个kv内部的分隔符
 * @return 拼接后的字符串
 */
STRING_TOOLS_API std::string JoinMap(const std::map<std::string, std::string>& kvmap, const std::string& vsep = "&",
                                     const std::string& msep = "=");

/**
 * @brief 向s中添加kv字段
 * @param s 待处理字符串
 * @param key key
 * @param val val
 * @param vsep 多个kv之间的分隔符
 * @param msep 单个kv内部的分隔符
 * @return 拼接后的字符串
 */
inline void SetParamsKV(std::string& s, const std::string& key, const std::string& val,
                        const std::string& vsep = "&", const std::string& msep = "=") {
  if (!s.empty()) s += vsep;
  s += (key + msep + val);
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
STRING_TOOLS_API std::string GetValueFromStrKV(const std::string& str, const std::string& key,
                                               const std::string& vsep = "&", const std::string& msep = "=",
                                               bool trimempty = true);

/**
 * @brief 分割字符串到vector
 * @param source 待处理字符串
 * @param separators 分隔符
 * @param cleanempty 是否去除空项
 * @param trimempty 是否对每项结果去除空格
 * @return 分割结果
 */
STRING_TOOLS_API std::vector<std::string> SplitToVec(const std::string& source, const std::string& separators,
                                                     bool cleanempty = true, bool trimempty = true);

/**
 * @brief 拼接vector到string
 * @param vec 待处理vector
 * @param separators 分隔符
 * @return 拼接后的字符串
 */
STRING_TOOLS_API std::string JoinVec(const std::vector<std::string>& vec, const std::string& separators);

/**
 * @brief 拼接set到string
 * @param st 待处理set
 * @param separators 分隔符
 * @return 拼接后的字符串
 */
STRING_TOOLS_API std::string JoinSet(const std::set<std::string>& st, const std::string& separators);

/**
 * @brief 比较版本，如6.1.1 6.2.8
 * @param ver1 版本1
 * @param ver2 版本2
 * @return 返回1是大于，返回0是相等，返回-1是小于
 */
STRING_TOOLS_API int CmpVersion(const std::string& ver1, const std::string& ver2);

/**
 * @brief 比较ios版本。去除掉版本字段中的“ios”字符串后再调用CmpVersion
 * @param ver1 版本1
 * @param ver2 版本2
 * @return 返回1是大于，返回0是相等，返回-1是小于
 */
STRING_TOOLS_API int IosCmpVersion(const std::string& ver1, const std::string& ver2);

/**
 * @brief 检查版本是否处于设定版本之间。需要保证start_ver<end_ver。若end_ver空，则设为999.9.9.9
 * @param ver 版本
 * @param start_ver 开始版本
 * @param end_ver 结束版本
 * @return 返回true是在传入的版本之间
 */
STRING_TOOLS_API bool CheckVersionInside(const std::string& ver, const std::string& start_ver,
                                         const std::string& end_ver);

/**
 * @brief 将一个字符串中指定字符串ov换为字符串nv
 * @param str 待替换字符串
 * @param ov 要被替换的子字符串
 * @param nv 要替换成的子字符串
 * @return 替换后的字符串
 */
STRING_TOOLS_API std::string& ReplaceString(std::string& str, const std::string& ov, const std::string& nv);

/**
 * @brief 判断字符串是否为数字和字母组成
 * @param str 待判断字符串
 * @return 是否为数字和字母组成
 */
STRING_TOOLS_API bool IsAlnumStr(const std::string& str);

/**
 * @brief 判断key字符串是否在list字符串中，如"123"是否在"123,456,789"中
 * 注意，key中不可包含分隔符，key不可为空，否则返回false
 * @param strlist list字符串
 * @param key key字符串
 * @param sep list的分隔符
 * @return key是否在list中
 */
STRING_TOOLS_API bool CheckIfInList(const std::string& strlist, const std::string& key, char sep = ',');

/**
 * @brief 获取map中的key的集合
 * @param ValType map中val的类型
 * @param m 输入map
 * @return key的集合
 */
template <typename ValType>
std::set<std::string> GetMapKeys(const std::map<std::string, ValType>& m) {
  std::set<std::string> re;
  for (const auto& it : m) re.insert(it.first);
  return re;
}

}  // namespace ytlib
