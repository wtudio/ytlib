#include <gtest/gtest.h>

#include "complex.hpp"
#include "math_def.h"
#include "matrix.hpp"
#include "ytlib/misc/stl_util.hpp"

namespace ytlib {

TEST(COMPLEX_TEST, BASE_test) {
  Complex c;
  EXPECT_DOUBLE_EQ(c.real, 0.0);
  EXPECT_DOUBLE_EQ(c.imag, 0.0);

  Complex c1 = {1.0, 2.0};
  EXPECT_DOUBLE_EQ(c1.real, 1.0);
  EXPECT_DOUBLE_EQ(c1.imag, 2.0);

  Complex c2(3.0, 4.0);
  EXPECT_DOUBLE_EQ(c2.real, 3.0);
  EXPECT_DOUBLE_EQ(c2.imag, 4.0);

  Complex c3(c2);
  EXPECT_DOUBLE_EQ(c3.real, 3.0);
  EXPECT_DOUBLE_EQ(c3.imag, 4.0);

  EXPECT_TRUE(c2 == c3);
  EXPECT_TRUE(c1 != c3);

  Complex c4 = -c3;
  EXPECT_DOUBLE_EQ(c4.real, -3.0);
  EXPECT_DOUBLE_EQ(c4.imag, -4.0);

  std::swap(c4, c3);
  EXPECT_DOUBLE_EQ(c4.real, 3.0);
  EXPECT_DOUBLE_EQ(c4.imag, 4.0);
  EXPECT_DOUBLE_EQ(c3.real, -3.0);
  EXPECT_DOUBLE_EQ(c3.imag, -4.0);

  Complex c5 = Complex<>::Conj(Complex(1.0, 1.0));
  EXPECT_DOUBLE_EQ(c5.real, 1.0);
  EXPECT_DOUBLE_EQ(c5.imag, -1.0);

  double abs_value = Complex(-3.0, 4.0).Len();
  EXPECT_DOUBLE_EQ(abs_value, 5.0);

  abs_value = abs(Complex(3.0, -4.0));
  EXPECT_DOUBLE_EQ(abs_value, 5.0);

  double angle_value = Complex(1.0, 0.0).Angle();
  EXPECT_DOUBLE_EQ(angle_value, 0.0);

  angle_value = Complex(0.0, 1.0).Angle();
  EXPECT_DOUBLE_EQ(angle_value, MATH_PI_2);

  Complex c6 = Complex<>::GenWithExpForm(2, MATH_PI_2);
  EXPECT_NEAR(c6.real, 0.0, 1e-6);
  EXPECT_DOUBLE_EQ(c6.imag, 2.0);

  c6.AssignWithExpForm(3, MATH_PI);
  EXPECT_DOUBLE_EQ(c6.real, -3.0);
  EXPECT_NEAR(c6.imag, 0.0, 1e-6);
}

TEST(COMPLEX_TEST, Add_test) {
  struct TestCase {
    std::string name;

    Complex<> a;
    Complex<> b;

    Complex<> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = {1.0, 2.0},
      .b = {3.0, 4.0},
      .want_result = {4.0, 6.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = {1.0, -1.0},
      .b = {1.0, 1.0},
      .want_result = {2.0, 0.0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].a + test_cases[ii].b;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].a += test_cases[ii].b;
    EXPECT_EQ(test_cases[ii].a, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, Sub_test) {
  struct TestCase {
    std::string name;

    Complex<> a;
    Complex<> b;

    Complex<> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = {1.0, 1.0},
      .b = {1.0, 1.0},
      .want_result = {0.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = {2.0, 3.0},
      .b = {1.0, 1.0},
      .want_result = {1.0, 2.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .a = {1.0, 1.0},
      .b = {2.0, 3.0},
      .want_result = {-1.0, -2.0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].a - test_cases[ii].b;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].a -= test_cases[ii].b;
    EXPECT_EQ(test_cases[ii].a, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, Multiply_test) {
  struct TestCase {
    std::string name;

    Complex<> a;
    Complex<> b;

    Complex<> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = {1.0, 1.0},
      .b = {1.0, 1.0},
      .want_result = {0.0, 2.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = {0.0, 1.0},
      .b = {1.0, 1.0},
      .want_result = {-1.0, 1.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = {1.0, 1.0},
      .b = {1.0, -1.0},
      .want_result = {2.0, 0.0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].a * test_cases[ii].b;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].a *= test_cases[ii].b;
    EXPECT_EQ(test_cases[ii].a, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, MultiplyNum_test) {
  struct TestCase {
    std::string name;

    Complex<> a;
    double b;

    Complex<> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = {1.0, 2.0},
      .b = 2,
      .want_result = {2.0, 4.0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].a * test_cases[ii].b;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].a *= test_cases[ii].b;
    EXPECT_EQ(test_cases[ii].a, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, Divide_test) {
  struct TestCase {
    std::string name;

    Complex<> a;
    Complex<> b;

    Complex<> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .a = {1.0, 1.0},
      .b = {1.0, 1.0},
      .want_result = {1.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .a = {-1.0, 1.0},
      .b = {1.0, 1.0},
      .want_result = {0.0, 1.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .a = {1.0, 2.0},
      .b = {1.0, 1.0},
      .want_result = {1.5, 0.5}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = test_cases[ii].a / test_cases[ii].b;
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;

    test_cases[ii].a /= test_cases[ii].b;
    EXPECT_EQ(test_cases[ii].a, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, Sqrt_test) {
  struct TestCase {
    std::string name;

    Complex<> value;

    Complex<> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .value = {0.0, 2.0},
      .want_result = {1.0, 1.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .value = {4.0, 0.0},
      .want_result = {2.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .value = {-4.0, 0.0},
      .want_result = {0.0, 2.0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Complex<>::Sqrt(test_cases[ii].value);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, Pow_test) {
  struct TestCase {
    std::string name;

    Complex<> value;
    uint32_t n;

    Complex<> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .value = {0.0, 0.0},
      .n = 10,
      .want_result = {0.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .value = {1.0, 0.0},
      .n = 10,
      .want_result = {1.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .value = {1.0, 1.0},
      .n = 4,
      .want_result = {-4.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .value = {1.0, 1.0},
      .n = 8,
      .want_result = {16.0, 0.0}});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .value = {1.0, 1.0},
      .n = 6,
      .want_result = {0.0, -8.0}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = Complex<>::Pow(test_cases[ii].value, test_cases[ii].n);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, ostream_test) {
  struct TestCase {
    std::string name;

    Complex<> value;

    std::string want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .value = {0.0, 2.0},
      .want_result = "0+2i"});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .value = {0.0, -2.0},
      .want_result = "0-2i"});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .value = {4.0, 0.0},
      .want_result = "4+0i"});
  test_cases.emplace_back(TestCase{
      .name = "case 4",
      .value = {-4.0, 0.0},
      .want_result = "-4+0i"});
  test_cases.emplace_back(TestCase{
      .name = "case 5",
      .value = {1.0, 1.0},
      .want_result = "1+1i"});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::stringstream ss;
    ss << test_cases[ii].value;
    EXPECT_STREQ(ss.str().c_str(), test_cases[ii].want_result.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, GetComplex_test) {
  struct TestCase {
    std::string name;

    uint32_t len;
    std::vector<double> in_vec;
    std::vector<Complex<double> > out_vec;

    std::vector<Complex<double> > want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 3,
      .in_vec = {1.0, 2.0, 3.0},
      .out_vec = std::vector<Complex<double> >(3),
      .want_result = {{1.0, 0.0}, {2.0, 0.0}, {3.0, 0.0}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    GetComplex(test_cases[ii].len, test_cases[ii].in_vec.data(), test_cases[ii].out_vec.data());
    EXPECT_EQ(test_cases[ii].out_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, ConjugateComplex_test) {
  struct TestCase {
    std::string name;

    uint32_t len;
    std::vector<Complex<double> > in_vec;

    std::vector<Complex<double> > want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 3,
      .in_vec = {{1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}},
      .want_result = {{1.0, -1.0}, {2.0, -2.0}, {3.0, -3.0}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    ConjugateComplex(test_cases[ii].len, test_cases[ii].in_vec.data());
    EXPECT_EQ(test_cases[ii].in_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, AbsComplex_test) {
  struct TestCase {
    std::string name;

    uint32_t len;
    std::vector<Complex<double> > in_vec;
    std::vector<double> out_vec;

    std::vector<double> want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 3,
      .in_vec = {{3.0, 4.0}, {5.0, 12.0}, {7.0, 24.0}},
      .out_vec = std::vector<double>(3),
      .want_result = {5, 13, 25}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    AbsComplex(test_cases[ii].len, test_cases[ii].in_vec.data(), test_cases[ii].out_vec.data());
    EXPECT_EQ(test_cases[ii].out_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(COMPLEX_TEST, FFT_test) {
  struct TestCase {
    std::string name;

    uint32_t N;
    std::vector<Complex<double> > in_vec;

    std::vector<Complex<double> > want_result;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .N = 16};
    test_case.in_vec.resize(test_case.N);
    test_case.want_result.resize(test_case.N);
    test_cases.emplace_back(std::move(test_case));
  }

  {
    TestCase test_case{
        .name = "case 2",
        .N = 16};
    double A = 2.0;
    uint32_t k = 2;

    test_case.in_vec.resize(test_case.N);
    for (uint32_t ii = 0; ii < test_case.N; ++ii) {
      test_case.in_vec[ii].AssignWithExpForm(A, k * ii * MATH_PI / test_case.N);
    }

    test_case.want_result.resize(test_case.N);
    test_case.want_result[k / 2] = Complex<double>(A * test_case.N, 0.0);

    test_cases.emplace_back(std::move(test_case));
  }

  {
    TestCase test_case{
        .name = "case 3",
        .N = 16};
    double A1 = 10.0;
    uint32_t k1 = 4;
    double A2 = 20.0;
    uint32_t k2 = 12;

    test_case.in_vec.resize(test_case.N);
    for (uint32_t ii = 0; ii < test_case.N; ++ii) {
      test_case.in_vec[ii] = Complex<double>::GenWithExpForm(A1, k1 * ii * MATH_PI / test_case.N) +
                             Complex<double>::GenWithExpForm(A2, k2 * ii * MATH_PI / test_case.N);
    }

    test_case.want_result.resize(test_case.N);
    test_case.want_result[k1 / 2] = Complex<double>(A1 * test_case.N, 0.0);
    test_case.want_result[k2 / 2] = Complex<double>(A2 * test_case.N, 0.0);

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::stringstream err_info;
    err_info << "Test " << test_cases[ii].name << " failed, index " << ii << "\n"
             << "in_vec : " << Vec2Str(test_cases[ii].in_vec) << "\n"
             << "want_result : " << Vec2Str(test_cases[ii].want_result);

    FFT(test_cases[ii].N, test_cases[ii].in_vec.data());
    for (uint32_t jj = 0; jj < test_cases[ii].N; ++jj) {
      EXPECT_NEAR(test_cases[ii].in_vec[jj].real, test_cases[ii].want_result[jj].real, 1e-6)
          << err_info.str();
      EXPECT_NEAR(test_cases[ii].in_vec[jj].imag, test_cases[ii].want_result[jj].imag, 1e-6)
          << err_info.str();
    }
  }
}

TEST(COMPLEX_TEST, IFFT_test) {
  struct TestCase {
    std::string name;

    uint32_t N;
    std::vector<Complex<double> > in_vec;

    std::vector<Complex<double> > want_result;
  };
  std::vector<TestCase> test_cases;
  {
    TestCase test_case{
        .name = "case 1",
        .N = 16};
    test_case.in_vec.resize(test_case.N);
    test_case.want_result.resize(test_case.N);
    test_cases.emplace_back(std::move(test_case));
  }

  {
    TestCase test_case{
        .name = "case 2",
        .N = 16};
    double A = 2.0;
    uint32_t k = 2;

    test_case.in_vec.resize(test_case.N);
    test_case.in_vec[k / 2] = Complex<double>(A * test_case.N, 0.0);

    test_case.want_result.resize(test_case.N);
    for (uint32_t ii = 0; ii < test_case.N; ++ii) {
      test_case.want_result[ii].AssignWithExpForm(A, k * ii * MATH_PI / test_case.N);
    }

    test_cases.emplace_back(std::move(test_case));
  }

  {
    TestCase test_case{
        .name = "case 3",
        .N = 16};
    double A1 = 10.0;
    uint32_t k1 = 4;
    double A2 = 20.0;
    uint32_t k2 = 12;

    test_case.in_vec.resize(test_case.N);
    test_case.in_vec[k1 / 2] = Complex<double>(A1 * test_case.N, 0.0);
    test_case.in_vec[k2 / 2] = Complex<double>(A2 * test_case.N, 0.0);

    test_case.want_result.resize(test_case.N);
    for (uint32_t ii = 0; ii < test_case.N; ++ii) {
      test_case.want_result[ii] = Complex<double>::GenWithExpForm(A1, k1 * ii * MATH_PI / test_case.N) +
                                  Complex<double>::GenWithExpForm(A2, k2 * ii * MATH_PI / test_case.N);
    }

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::stringstream err_info;
    err_info << "Test " << test_cases[ii].name << " failed, index " << ii << "\n"
             << "in_vec : " << Vec2Str(test_cases[ii].in_vec) << "\n"
             << "want_result : " << Vec2Str(test_cases[ii].want_result);

    IFFT(test_cases[ii].N, test_cases[ii].in_vec.data());
    for (uint32_t jj = 0; jj < test_cases[ii].N; ++jj) {
      EXPECT_NEAR(test_cases[ii].in_vec[jj].real, test_cases[ii].want_result[jj].real, 1e-6)
          << err_info.str();
      EXPECT_NEAR(test_cases[ii].in_vec[jj].imag, test_cases[ii].want_result[jj].imag, 1e-6)
          << err_info.str();
    }
  }
}

TEST(COMPLEX_TEST, FFTShift_test) {
  struct TestCase {
    std::string name;

    uint32_t len;
    std::vector<Complex<double> > in_vec;

    std::vector<Complex<double> > want_result;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .len = 4,
      .in_vec = {{1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}, {4.0, 4.0}},
      .want_result = {{3.0, 3.0}, {4.0, 4.0}, {1.0, 1.0}, {2.0, 2.0}}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .len = 5,
      .in_vec = {{1.0, 1.0}, {2.0, 2.0}, {3.0, 3.0}, {4.0, 4.0}, {5.0, 5.0}},
      .want_result = {{3.0, 3.0}, {4.0, 4.0}, {1.0, 1.0}, {2.0, 2.0}, {5.0, 5.0}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    FFTShift(test_cases[ii].len, test_cases[ii].in_vec.data());
    EXPECT_EQ(test_cases[ii].in_vec, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

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
