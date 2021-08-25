/**
 * @file stl_util.hpp
 * @author WT
 * @brief stl容器相关工具
 * @note stl容器相关工具
 * @date 2021-08-09
 */

#pragma once

#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace ytlib {

/**
 * @brief 打印vector。自定义打印方式
 * 
 * @tparam T vector模板参数
 * @param v 待打印vector
 * @param f 打印方法
 * @return std::string 结果字符串
 */
template <typename T>
std::string Vec2Str(const std::vector<T>& v, std::function<std::string(const T&)> f) {
  std::stringstream ss;
  ss << "vec size = " << v.size() << '\n';
  if (!f) return ss.str();

  const size_t MAX_LINE_LEN = 32;

  size_t ct = 0;
  for (auto& itr : v) {
    std::string obj_str = f(itr);
    if (obj_str.empty()) obj_str = "<empty string>";

    ss << "[index=" << ct << "]:";
    if (obj_str.length() > MAX_LINE_LEN || obj_str.find('\n') != std::string::npos) {
      ss << '\n';
    }

    ss << obj_str << '\n';

    ++ct;
  }
  return ss.str();
}

/**
 * @brief 打印vector。使用T的 << 方法
 * 
 * @tparam T vector模板参数
 * @param v 待打印vector
 * @return std::string 结果字符串
 */
template <typename T>
std::string Vec2Str(const std::vector<T>& v) {
  std::function<std::string(const T&)> f = [](const T& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
  };
  return Vec2Str(v, f);
}

/**
 * @brief 打印set。自定义打印方式
 * 
 * @tparam T set模板参数
 * @param s 待打印set
 * @param f 打印方法
 * @return std::string 结果字符串
 */
template <typename T>
std::string Set2Str(const std::set<T>& s, std::function<std::string(const T&)> f) {
  std::stringstream ss;
  ss << "set size = " << s.size() << '\n';
  if (!f) return ss.str();

  const size_t MAX_LINE_LEN = 32;

  size_t ct = 0;
  for (auto& itr : s) {
    std::string obj_str = f(itr);
    if (obj_str.empty()) obj_str = "<empty string>";

    ss << "[index=" << ct << "]:";
    if (obj_str.length() > MAX_LINE_LEN || obj_str.find('\n') != std::string::npos) {
      ss << '\n';
    }

    ss << obj_str << '\n';

    ++ct;
  }
  return ss.str();
}

/**
 * @brief 打印set。使用T的 << 方法
 * 
 * @tparam T set模板参数
 * @param s 待打印set
 * @return std::string 结果字符串
 */
template <typename T>
std::string Set2Str(const std::set<T>& s) {
  std::function<std::string(const T&)> f = [](const T& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
  };
  return Set2Str(s, f);
}

/**
 * @brief 打印map。自定义打印方式
 * 
 * @tparam KeyType map中key类型
 * @tparam ValType map中val类型
 * @param m 待打印map
 * @param fkey key打印方法
 * @param fval val打印方法
 * @return std::string 结果字符串
 */
template <typename KeyType, typename ValType>
std::string Map2Str(const std::map<KeyType, ValType>& m,
                    std::function<std::string(const KeyType&)> fkey,
                    std::function<std::string(const ValType&)> fval) {
  std::stringstream ss;
  ss << "map size = " << m.size() << '\n';
  if (!fkey) return ss.str();

  const size_t MAX_LINE_LEN = 32;

  size_t ct = 0;
  for (auto& itr : m) {
    std::string key_str = fkey(itr.first);
    if (key_str.empty()) key_str = "<empty string>";

    std::string val_str;
    if (fval) {
      val_str = fval(itr.second);
      if (val_str.empty()) val_str = "<empty string>";
    } else {
      val_str = "<unable to print>";
    }

    ss << "[index=" << ct << "]:\n  [key]:";
    if (key_str.length() > MAX_LINE_LEN || key_str.find('\n') != std::string::npos) {
      ss << '\n';
    }

    ss << key_str << "\n  [val]:";

    if (val_str.length() > MAX_LINE_LEN || val_str.find('\n') != std::string::npos) {
      ss << '\n';
    }

    ss << val_str << '\n';

    ++ct;
  }
  return ss.str();
}

/**
 * @brief 打印map。使用KeyType、ValType的 << 方法
 * 
 * @tparam KeyType map中key类型
 * @tparam ValType map中val类型
 * @param m 待打印map
 * @return std::string 结果字符串
 */
template <typename KeyType, typename ValType>
std::string Map2Str(const std::map<KeyType, ValType>& m) {
  std::function<std::string(const KeyType&)> fkey = [](const KeyType& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
  };
  std::function<std::string(const ValType&)> fval = [](const ValType& obj) {
    std::stringstream ss;
    ss << obj;
    return ss.str();
  };
  return Map2Str(m, fkey, fval);
}

/**
 * @brief 判断两个vector是否相等
 * 
 * @tparam T vector模板参数，需支持!=运算
 * @param[in] vec1 
 * @param[in] vec2 
 * @return true 相等
 * @return false 不相等
 */
template <typename T>
bool CheckVectorEqual(const std::vector<T>& vec1, const std::vector<T>& vec2) {
  if (vec1.size() != vec2.size()) return false;
  for (size_t ii = 0; ii < vec1.size(); ++ii) {
    if (vec1[ii] != vec2[ii]) return false;
  }
  return true;
}

/**
 * @brief 判断两个set是否相等
 * 
 * @tparam T set模板参数，需支持!=运算
 * @param[in] set1 
 * @param[in] set2 
 * @return true 相等
 * @return false 不相等
 */
template <typename T>
bool CheckSetEqual(const std::set<T>& set1, const std::set<T>& set2) {
  if (set1.size() != set2.size()) return false;
  auto itr1 = set1.begin();
  auto itr2 = set2.begin();
  for (size_t ii = 0; ii < set1.size(); ++ii) {
    if (*itr1 != *itr2) return false;
    ++itr1;
    ++itr2;
  }
  return true;
}

/**
 * @brief 判断两个map是否相等
 * 
 * @tparam KeyType map模板参数，需支持!=运算
 * @tparam ValType map模板参数，需支持!=运算
 * @param[in] map1 
 * @param[in] map2 
 * @return true 相等
 * @return false 不相等
 */
template <typename KeyType, typename ValType>
bool CheckMapEqual(const std::map<KeyType, ValType>& map1, const std::map<KeyType, ValType>& map2) {
  if (map1.size() != map2.size()) return false;
  auto itr1 = map1.begin();
  auto itr2 = map2.begin();
  for (size_t ii = 0; ii < map1.size(); ++ii) {
    if ((itr1->first != itr2->first) || (itr1->second != itr2->second)) return false;
    ++itr1;
    ++itr2;
  }
  return true;
}

}  // namespace ytlib
