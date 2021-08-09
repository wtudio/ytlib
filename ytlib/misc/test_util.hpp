/**
 * @file test_util.hpp
 * @author WT
 * @brief 测试相关工具
 * @note 测试相关工具
 * @date 2021-08-09
 */

#pragma once

#include <map>
#include <set>
#include <vector>

namespace ytlib {

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
  for (std::size_t ii = 0; ii < vec1.size(); ++ii) {
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
  for (std::size_t ii = 0; ii < set1.size(); ++ii) {
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
  for (std::size_t ii = 0; ii < map1.size(); ++ii) {
    if ((itr1->first != itr2->first) || (itr1->second != itr2->second)) return false;
    ++itr1;
    ++itr2;
  }
  return true;
}

}  // namespace ytlib
