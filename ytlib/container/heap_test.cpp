#include <gtest/gtest.h>

#include "heap.hpp"

namespace ytlib {

TEST(HEAP_TEST, BASE_test) {
  std::vector<int> test_vec{7, 9, 2, 6, 4, 8, 1, 3, 10, 5};

  struct TestCase {
    std::string name;

    Heap<int> heap;

    bool want_is_min_heap;
    std::vector<int> want_content;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .heap = Heap<int>(),
      .want_is_min_heap = true,
      .want_content = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .heap = Heap<int>(test_vec),
      .want_is_min_heap = true,
      .want_content = {1, 3, 2, 6, 4, 8, 7, 9, 10, 5}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .heap = Heap<int>(test_vec.data(), test_vec.size()),
      .want_is_min_heap = true,
      .want_content = {1, 3, 2, 6, 4, 8, 7, 9, 10, 5}});
  {
    TestCase test_case{
        .name = "case 4",
        .heap = Heap<int>(),
        .want_is_min_heap = true,
        .want_content = {1, 3, 2, 6, 4, 8, 7, 9, 10, 5}};
    test_case.heap.Assign(test_vec.data(), test_vec.size());
    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(test_cases[ii].heap.is_min_heap_, test_cases[ii].want_is_min_heap)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].heap.container_, test_cases[ii].want_content)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(HEAP_TEST, Push_test) {
  std::vector<int> test_vec{7, 9, 2, 6, 4, 8, 1, 3, 10, 5};

  struct TestCase {
    std::string name;

    Heap<int> heap;
    int push_num;

    bool want_is_min_heap;
    std::vector<int> want_content;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .heap = Heap<int>(test_vec),
      .push_num = 0,
      .want_is_min_heap = true,
      .want_content = {0, 1, 2, 6, 3, 8, 7, 9, 10, 5, 4}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .heap = Heap<int>(test_vec),
      .push_num = 5,
      .want_is_min_heap = true,
      .want_content = {1, 3, 2, 6, 4, 8, 7, 9, 10, 5, 5}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .heap = Heap<int>(test_vec),
      .push_num = 100,
      .want_is_min_heap = true,
      .want_content = {1, 3, 2, 6, 4, 8, 7, 9, 10, 5, 100}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    test_cases[ii].heap.Push(test_cases[ii].push_num);
    EXPECT_EQ(test_cases[ii].heap.is_min_heap_, test_cases[ii].want_is_min_heap)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].heap.container_, test_cases[ii].want_content)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(HEAP_TEST, Pop_test) {
  std::vector<int> test_vec{7, 9, 2, 6, 4, 8, 1, 3, 10, 5};

  struct TestCase {
    std::string name;

    Heap<int> heap;

    bool want_is_min_heap;
    std::vector<int> want_content;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .heap = Heap<int>(test_vec),
      .want_is_min_heap = true,
      .want_content = {2, 3, 5, 6, 4, 8, 7, 9, 10},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .heap = Heap<int>(),
      .want_is_min_heap = true,
      .want_content = {},
      .want_exp = true});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    bool get_exp = false;
    try {
      test_cases[ii].heap.Pop();
      EXPECT_EQ(test_cases[ii].heap.is_min_heap_, test_cases[ii].want_is_min_heap)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
      EXPECT_EQ(test_cases[ii].heap.container_, test_cases[ii].want_content)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(HEAP_TEST, Sort_test) {
  struct TestCase {
    std::string name;

    Heap<int> heap;

    bool want_is_min_heap;
    std::vector<int> want_content;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .heap = Heap<int>(std::vector<int>{7, 9, 2, 6, 4, 8, 1, 3, 10, 5}),
      .want_is_min_heap = false,
      .want_content = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .heap = Heap<int>(std::vector<int>{7, -5, 9, -7, 2, 6, -6, 4, 8, 1, 3, -8, 10, 5, -9}),
      .want_is_min_heap = false,
      .want_content = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1, -5, -6, -7, -8, -9}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    test_cases[ii].heap.Sort();
    EXPECT_EQ(test_cases[ii].heap.is_min_heap_, test_cases[ii].want_is_min_heap)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(test_cases[ii].heap.container_, test_cases[ii].want_content)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
