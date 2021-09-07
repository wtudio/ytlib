/**
 * @file rule_tool.hpp
 * @author WT
 * @brief 规则工具
 * @note 规则工具
 * @date 2021-09-06
 */
#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace ytlib {

/**
 * @brief 规则执行工具
 * @note 一条规则就是一个std::function<bool()>，返回true则代表命中了该条规则。
 * 按照指定的顺序执行规则，当命中某条规则后则跳过后续规则，并返回命中规则的名称。
 * @param rule_order 规则执行顺序
 * @param rule_map 规则名称-规则内容
 * @return std::string 命中的规则名称。都没有命中则返回空
 */
inline std::string RunRules(const std::vector<std::string>& rule_order,
                            const std::map<std::string, std::function<bool()> >& rule_map) {
  for (const auto& rule_name : rule_order) {
    auto rule_itr = rule_map.find(rule_name);
    if (rule_itr != rule_map.end() && rule_itr->second())
      return rule_name;
  }
  return "";
}

}  // namespace ytlib
