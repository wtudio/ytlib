#include <gtest/gtest.h>

#include "rule_tool.hpp"

namespace ytlib {

TEST(RULE_TOOL_TEST, RunRules) {
  int rule_result = 0;

  std::map<std::string, std::function<bool()> > rule_map;
  rule_map.emplace("r1", [&]() {
    rule_result = 1;
    return false;
  });
  rule_map.emplace("r2", [&]() {
    rule_result = 2;
    return true;
  });
  rule_map.emplace("r3", [&]() {
    rule_result = 3;
    return true;
  });

  std::string hit_rule = RunRules(std::vector<std::string>{"r1", "r2", "r3"}, rule_map);
  EXPECT_STREQ(hit_rule.c_str(), "r2");
  EXPECT_EQ(rule_result, 2);
}

}  // namespace ytlib
