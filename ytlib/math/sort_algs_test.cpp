#include <gtest/gtest.h>

#include "sort_algs.hpp"

namespace ytlib {

TEST(SORT_ALGS_TEST, SortObj_test) {
  SortObj obj1(1);
  SortObj obj2(1);
  SortObj obj3(2);

  EXPECT_TRUE(obj1 < obj3);
  EXPECT_TRUE(obj3 > obj1);
  EXPECT_TRUE(obj1 <= obj3);
  EXPECT_TRUE(obj3 >= obj1);
  EXPECT_TRUE(obj1 == obj2);
  EXPECT_FALSE(obj1 != obj2);
  EXPECT_TRUE(obj1 != obj3);
  EXPECT_FALSE(obj1 == obj3);
}

TEST(SORT_ALGS_TEST, BubbleSort_test) {
  struct TestCase {
    std::string name;

    size_t len;
    std::vector<uint32_t> data_vec;

    std::vector<uint32_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 10,
      .data_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .len = 10,
      .data_vec = {7, 3, 9, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .len = 10,
      .data_vec = {7, 3, 5, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 5, 6, 7, 8}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    BubbleSort(test_cases[ii].data_vec.data(), test_cases[ii].len);
    EXPECT_EQ(test_cases[ii].data_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(SORT_ALGS_TEST, MergeSort_test) {
  struct TestCase {
    std::string name;

    size_t len;
    std::vector<uint32_t> data_vec;

    std::vector<uint32_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 10,
      .data_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .len = 10,
      .data_vec = {7, 3, 9, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .len = 10,
      .data_vec = {7, 3, 5, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 5, 6, 7, 8}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    MergeSort(test_cases[ii].data_vec.data(), test_cases[ii].len);
    EXPECT_EQ(test_cases[ii].data_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(SORT_ALGS_TEST, MergeSort2_test) {
  struct TestCase {
    std::string name;

    size_t len;
    std::vector<uint32_t> data_vec;

    std::vector<uint32_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 10,
      .data_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .len = 10,
      .data_vec = {7, 3, 9, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .len = 10,
      .data_vec = {7, 3, 5, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 5, 6, 7, 8}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    MergeSort2(test_cases[ii].data_vec.data(), test_cases[ii].len);
    EXPECT_EQ(test_cases[ii].data_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(SORT_ALGS_TEST, QuickSort_test) {
  struct TestCase {
    std::string name;

    size_t len;
    std::vector<uint32_t> data_vec;

    std::vector<uint32_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 10,
      .data_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .len = 10,
      .data_vec = {7, 3, 9, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .len = 10,
      .data_vec = {7, 3, 5, 6, 1, 5, 0, 2, 8, 4},
      .want_result = {0, 1, 2, 3, 4, 5, 5, 6, 7, 8}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    QuickSort(test_cases[ii].data_vec.data(), test_cases[ii].len);
    EXPECT_EQ(test_cases[ii].data_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(SORT_ALGS_TEST, BinarySearch_test) {
  struct TestCase {
    std::string name;

    size_t len;
    std::vector<uint32_t> data_vec;
    uint32_t key;

    size_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 10,
      .data_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
      .key = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .len = 10,
      .data_vec = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4},
      .key = 2,
      .want_result = 4});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .len = 10,
      .data_vec = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      .key = 1,
      .want_result = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .len = 0,
      .data_vec = {},
      .key = 1,
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = BinarySearch(test_cases[ii].data_vec.data(), test_cases[ii].len, test_cases[ii].key);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(SORT_ALGS_TEST, BinarySearchLast_test) {
  struct TestCase {
    std::string name;

    size_t len;
    std::vector<uint32_t> data_vec;
    uint32_t key;

    size_t want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 10,
      .data_vec = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9},
      .key = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .len = 10,
      .data_vec = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4},
      .key = 2,
      .want_result = 6});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .len = 10,
      .data_vec = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
      .key = 1,
      .want_result = 10});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .len = 0,
      .data_vec = {},
      .key = 1,
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = BinarySearchLast(test_cases[ii].data_vec.data(), test_cases[ii].len, test_cases[ii].key);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib