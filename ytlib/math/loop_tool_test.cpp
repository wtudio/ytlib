#include <gtest/gtest.h>

#include "loop_tool.hpp"

namespace ytlib {

TEST(LOOP_TOOL_TEST, BASE_test) {
  struct TestCase {
    std::string name;

    std::vector<uint32_t> input_up_vec;

    uint64_t want_result_ct;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .input_up_vec = {},
      .want_result_ct = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .input_up_vec = {2, 3, 4},
      .want_result_ct = 23});  // 2*3*4-1

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    TestCase& cur_test_case = test_cases[ii];
    uint64_t ct = 0;
    LoopTool lt(cur_test_case.input_up_vec);
    while (++lt) {
      ++ct;
    }
    EXPECT_EQ(ct, cur_test_case.want_result_ct);
  }
}

}  // namespace ytlib
