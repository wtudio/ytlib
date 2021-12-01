#include <gtest/gtest.h>

#include "math_def.h"
#include "matrix.hpp"

namespace ytlib {

TEST(MATRIX_TEST, BASE_test) {
  const uint32_t kRow = 3;
  const uint32_t kCol = 4;

  Matrix_i32 m1(kRow, kCol);
  m1.val[0][0] = 100;
  EXPECT_EQ(m1.val[0][0], 100);

  Matrix_i32 m2(m1);
  EXPECT_EQ(m2.val[0][0], 100);

  Matrix_i32 m3;
  m3 = m1;
  EXPECT_EQ(m3.val[0][0], 100);

  Matrix_i32 m4(std::move(m2));
  EXPECT_EQ(m4.val[0][0], 100);
  EXPECT_EQ(m2.val, nullptr);

  Matrix_i32 m5;
  m5 = std::move(m3);
  EXPECT_EQ(m5.val[0][0], 100);
  EXPECT_EQ(m3.val, nullptr);

  EXPECT_TRUE(m4 == m5);
  EXPECT_FALSE(m4 != m5);

  m4.val[0][1] = 200;
  EXPECT_TRUE(m4 != m5);
  EXPECT_FALSE(m4 == m5);

  Matrix_i32 m4_1 = m4;
  Matrix_i32 m5_1 = m5;
  swap(m4_1, m5_1);
  EXPECT_EQ(m4_1, m5);
  EXPECT_EQ(m5_1, m4);
}

TEST(MATRIX_TEST, DATA_test) {
  const uint32_t kRow = 2;
  const uint32_t kCol = 3;
  Matrix_i32 target_m(kRow, kCol);
  target_m.val[0][0] = 1;
  target_m.val[0][1] = 2;
  target_m.val[0][2] = 3;
  target_m.val[1][0] = 4;
  target_m.val[1][1] = 5;
  target_m.val[1][2] = 6;

  Matrix_i32 m1{kRow, kCol, {1, 2, 3, 4, 5, 6}};
  EXPECT_EQ(m1, target_m);

  Matrix_i32 m2{kRow, kCol, {{1, 2, 3}, {4, 5, 6}}};
  EXPECT_EQ(m2, target_m);

  Matrix_i32 m3(kRow, kCol);
  m3.Assgin(std::vector<int32_t>{1, 2, 3, 4, 5, 6});
  EXPECT_EQ(m3, target_m);

  Matrix_i32 m4(kRow, kCol);
  m4.Assgin(std::vector<std::vector<int32_t> >{{1, 2, 3}, {4, 5, 6}});
  EXPECT_EQ(m4, target_m);

  Matrix_i32 m5{kRow, kCol, {1, 2, 3, 4, 5, 6, 7}};
  EXPECT_EQ(m5, target_m);

  Matrix_i32 m6{kRow, kCol, {{1, 2, 3, 4}, {4, 5, 6, 7, 8}, {7}}};
  EXPECT_EQ(m6, target_m);

  Matrix_i32 m7(target_m);
  m7.Zero();
  EXPECT_EQ(m7, Matrix_i32(kRow, kCol));

  Matrix_i32 m8 = -target_m;
  Matrix_i32 target_m8{kRow, kCol, {{-1, -2, -3}, {-4, -5, -6}}};
  EXPECT_EQ(m8, target_m8);

  Matrix_i32 m9 = ~target_m;
  Matrix_i32 target_m9{kCol, kRow, {{1, 4}, {2, 5}, {3, 6}}};
  EXPECT_EQ(m9, target_m9);

  Matrix_i32 m10 = Matrix_i32::Eye(1, 3);
  Matrix_i32 target_m10{3, 3, {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}}};
  EXPECT_EQ(m10, target_m10);
}

TEST(MATRIX_TEST, GetData_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    uint32_t row_begin;
    uint32_t col_begin;
    uint32_t row_end;
    uint32_t col_end;

    std::vector<int32_t> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 0,
      .col_end = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 0,
      .col_end = 0,
      .want_result = {1}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 100,
      .col_end = 100,
      .want_result = {1, 2, 3, 4, 5, 6, 7, 8, 9}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 3,
      .col_begin = 3,
      .row_end = 100,
      .col_end = 100,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 1,
      .col_begin = 1,
      .row_end = 100,
      .col_end = 100,
      .want_result = {5, 6, 8, 9}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].M.GetData(test_cases[ii].row_begin, test_cases[ii].col_begin,
                                        test_cases[ii].row_end, test_cases[ii].col_end);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, GetMat_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    uint32_t row_begin;
    uint32_t col_begin;
    uint32_t row_end;
    uint32_t col_end;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 0,
      .col_end = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 0,
      .col_end = 0,
      .want_result = {1, 1, std::vector<int32_t>{1}}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 100,
      .col_end = 100,
      .want_result = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 3,
      .col_begin = 3,
      .row_end = 100,
      .col_end = 100,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .row_begin = 1,
      .col_begin = 1,
      .row_end = 100,
      .col_end = 100,
      .want_result = {2, 2, {{5, 6}, {8, 9}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].M.GetMat(test_cases[ii].row_begin, test_cases[ii].col_begin,
                                       test_cases[ii].row_end, test_cases[ii].col_end);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, SetMat_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    Matrix_i32 input_M;
    uint32_t row_begin;
    uint32_t col_begin;
    uint32_t row_end;
    uint32_t col_end;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .input_M = {},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 0,
      .col_end = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .input_M = {},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 1,
      .col_end = 1,
      .want_result = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .input_M = {1, 1, std::vector<int32_t>{100}},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 1,
      .col_end = 1,
      .want_result = {3, 3, {{100, 2, 3}, {4, 5, 6}, {7, 8, 9}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .input_M = {3, 3, {{100, 200, 300}, {400, 500, 600}, {700, 800, 900}}},
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 1,
      .col_end = 1,
      .want_result = {3, 3, {{100, 200, 3}, {400, 500, 6}, {7, 8, 9}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    test_cases[ii].M.SetMat(test_cases[ii].input_M,
                            test_cases[ii].row_begin, test_cases[ii].col_begin,
                            test_cases[ii].row_end, test_cases[ii].col_end);
    EXPECT_EQ(test_cases[ii].M, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, SetVal_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    int32_t in_val;
    uint32_t row_begin;
    uint32_t col_begin;
    uint32_t row_end;
    uint32_t col_end;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .in_val = 123,
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 0,
      .col_end = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .in_val = 123,
      .row_begin = 0,
      .col_begin = 0,
      .row_end = 1,
      .col_end = 1,
      .want_result = {3, 3, {{123, 123, 3}, {123, 123, 6}, {7, 8, 9}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .in_val = 123,
      .row_begin = 2,
      .col_begin = 2,
      .row_end = 100,
      .col_end = 100,
      .want_result = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 123}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    test_cases[ii].M.SetVal(test_cases[ii].in_val,
                            test_cases[ii].row_begin, test_cases[ii].col_begin,
                            test_cases[ii].row_end, test_cases[ii].col_end);
    EXPECT_EQ(test_cases[ii].M, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, SetDiag_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    int32_t in_val;
    uint32_t idx_begin;
    uint32_t idx_end;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .in_val = 123,
      .idx_begin = 0,
      .idx_end = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .in_val = 123,
      .idx_begin = 0,
      .idx_end = 0,
      .want_result = {3, 3, {{123, 2, 3}, {4, 5, 6}, {7, 8, 9}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .in_val = 123,
      .idx_begin = 0,
      .idx_end = 100,
      .want_result = {3, 3, {{123, 2, 3}, {4, 123, 6}, {7, 8, 123}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .M = {4, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}}},
      .in_val = 123,
      .idx_begin = 0,
      .idx_end = 100,
      .want_result = {4, 3, {{123, 2, 3}, {4, 123, 6}, {7, 8, 123}, {10, 11, 12}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    test_cases[ii].M.SetDiag(test_cases[ii].in_val,
                             test_cases[ii].idx_begin, test_cases[ii].idx_end);
    EXPECT_EQ(test_cases[ii].M, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, SetDiag2_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    std::vector<int32_t> input_vec;
    uint32_t idx_begin;
    uint32_t idx_end;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .input_vec = {123, 456},
      .idx_begin = 0,
      .idx_end = 0,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .input_vec = {123, 456},
      .idx_begin = 0,
      .idx_end = 0,
      .want_result = {3, 3, {{123, 2, 3}, {4, 5, 6}, {7, 8, 9}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {3, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}},
      .input_vec = {123, 456},
      .idx_begin = 0,
      .idx_end = 100,
      .want_result = {3, 3, {{123, 2, 3}, {4, 456, 6}, {7, 8, 9}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .M = {4, 3, {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12}}},
      .input_vec = {123, 456, 789, 123},
      .idx_begin = 0,
      .idx_end = 100,
      .want_result = {4, 3, {{123, 2, 3}, {4, 456, 6}, {7, 8, 789}, {10, 11, 12}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    test_cases[ii].M.SetDiag(test_cases[ii].input_vec,
                             test_cases[ii].idx_begin, test_cases[ii].idx_end);
    EXPECT_EQ(test_cases[ii].M, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, Add_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M1;
    Matrix_i32 M2;

    Matrix_i32 want_result;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M1 = {},
      .M2 = {},
      .want_result = {},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M1 = {},
      .M2 = {1, 1},
      .want_result = {},
      .want_exp = true});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M1 = {3, 3, {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}}},
      .M2 = {3, 3, {{10, 10, 10}, {20, 20, 20}, {30, 30, 30}}},
      .want_result = {3, 3, {{11, 11, 11}, {22, 22, 22}, {33, 33, 33}}},
      .want_exp = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    bool get_exp = false;
    try {
      auto ret = test_cases[ii].M1 + test_cases[ii].M2;
      EXPECT_EQ(ret, test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;

      test_cases[ii].M1 += test_cases[ii].M2;
      EXPECT_EQ(test_cases[ii].M1, test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, Sub_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M1;
    Matrix_i32 M2;

    Matrix_i32 want_result;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M1 = {},
      .M2 = {},
      .want_result = {},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M1 = {},
      .M2 = {1, 1},
      .want_result = {},
      .want_exp = true});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M1 = {3, 3, {{1, 1, 1}, {2, 2, 2}, {3, 3, 3}}},
      .M2 = {3, 3, {{10, 10, 10}, {20, 20, 20}, {30, 30, 30}}},
      .want_result = {3, 3, {{-9, -9, -9}, {-18, -18, -18}, {-27, -27, -27}}},
      .want_exp = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    bool get_exp = false;
    try {
      auto ret = test_cases[ii].M1 - test_cases[ii].M2;
      EXPECT_EQ(ret, test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;

      test_cases[ii].M1 -= test_cases[ii].M2;
      EXPECT_EQ(test_cases[ii].M1, test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, Multiply_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M1;
    Matrix_i32 M2;

    Matrix_i32 want_result;
    bool want_exp;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M1 = {},
      .M2 = {},
      .want_result = {},
      .want_exp = false});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M1 = {2, 2},
      .M2 = {3, 3},
      .want_result = {},
      .want_exp = true});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M1 = {2, 3, {{1, 1, 1}, {2, 2, 2}}},
      .M2 = {3, 4, {{3, 3, 3, 3}, {4, 4, 4, 4}, {5, 5, 5, 5}}},
      .want_result = {2, 4, {{12, 12, 12, 12}, {24, 24, 24, 24}}},
      .want_exp = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    bool get_exp = false;
    try {
      auto ret = test_cases[ii].M1 * test_cases[ii].M2;
      EXPECT_EQ(ret, test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;

      test_cases[ii].M1 *= test_cases[ii].M2;
      EXPECT_EQ(test_cases[ii].M1, test_cases[ii].want_result)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    } catch (const std::exception&) {
      get_exp = true;
    }
    EXPECT_EQ(get_exp, test_cases[ii].want_exp)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, MultiplyNum_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    int32_t in_val;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .in_val = 2,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {2, 2},
      .in_val = 2,
      .want_result = {2, 2}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {2, 3, {{1, 1, 1}, {2, 2, 2}}},
      .in_val = 2,
      .want_result = {2, 3, {{2, 2, 2}, {4, 4, 4}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].M * test_cases[ii].in_val;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].M *= test_cases[ii].in_val;
    EXPECT_EQ(test_cases[ii].M, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, DivideNum_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 M;
    int32_t in_val;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .M = {},
      .in_val = 2,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .M = {2, 2},
      .in_val = 2,
      .want_result = {2, 2}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .M = {2, 3, {{2, 2, 2}, {4, 4, 4}}},
      .in_val = 2,
      .want_result = {2, 3, {{1, 1, 1}, {2, 2, 2}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].M / test_cases[ii].in_val;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].M /= test_cases[ii].in_val;
    EXPECT_EQ(test_cases[ii].M, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, Pow_test) {
  struct TestCase {
    std::string name;

    int32_t in_val;
    Matrix_i32 M;
    uint32_t n;

    Matrix_i32 want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .in_val = 1,
      .M = {},
      .n = 10,
      .want_result = {}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .in_val = 1,
      .M = {2, 2, {{1, 0}, {0, 1}}},
      .n = 10,
      .want_result = {2, 2, {{1, 0}, {0, 1}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .in_val = 1,
      .M = {2, 2, {{2, 0}, {0, 2}}},
      .n = 10,
      .want_result = {2, 2, {{1024, 0}, {0, 1024}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Matrix_i32::Pow(test_cases[ii].in_val, test_cases[ii].M, test_cases[ii].n);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, ostream_test) {
  struct TestCase {
    std::string name;

    Matrix_i32 value;

    std::string want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .value = {},
      .want_result = "[empty matrix]"});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .value = {1, 0},
      .want_result = "[empty matrix]"});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .value = {2, 3},
      .want_result = R"str([row 2, col 3]
0	0	0
0	0	0
)str"});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .value = {2, 3, {{1, 2, 3}, {4, 5, 6}}},
      .want_result = R"str([row 2, col 3]
1	2	3
4	5	6
)str"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::stringstream ss;
    ss << test_cases[ii].value;
    EXPECT_STREQ(ss.str().c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATRIX_TEST, ROTMAT_test) {
  struct TestCase {
    std::string name;

    double angle;

    Matrix want_result_rotX;
    Matrix want_result_rotY;
    Matrix want_result_rotZ;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .angle = 0.0,
      .want_result_rotX = {3, 3, {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}},
      .want_result_rotY = {3, 3, {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}},
      .want_result_rotZ = {3, 3, {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}}}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .angle = MATH_PI_2,
      .want_result_rotX = {3, 3, {{1.0, 0.0, 0.0}, {0.0, 0.0, -1.0}, {0.0, 1.0, 0.0}}},
      .want_result_rotY = {3, 3, {{0.0, 0.0, 1.0}, {0.0, 1.0, 0.0}, {-1.0, 0.0, 0.0}}},
      .want_result_rotZ = {3, 3, {{0.0, -1.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 0.0, 1.0}}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret_x = RotMatX(test_cases[ii].angle);

    for (size_t jj = 0; jj < 9; ++jj) {
      EXPECT_NEAR(ret_x.val[0][jj], test_cases[ii].want_result_rotX.val[0][jj], 1e-6)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    }

    auto ret_y = RotMatY(test_cases[ii].angle);
    for (size_t jj = 0; jj < 9; ++jj) {
      EXPECT_NEAR(ret_y.val[0][jj], test_cases[ii].want_result_rotY.val[0][jj], 1e-6)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    }

    auto ret_z = RotMatZ(test_cases[ii].angle);
    for (size_t jj = 0; jj < 9; ++jj) {
      EXPECT_NEAR(ret_z.val[0][jj], test_cases[ii].want_result_rotZ.val[0][jj], 1e-6)
          << "Test " << test_cases[ii].name << " failed, index " << ii;
    }
  }
}

}  // namespace ytlib
