#include <gtest/gtest.h>

#include "complex.hpp"
#include "math_def.h"

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
    TestCase& cur_test_case = test_cases[ii];
    auto ret = cur_test_case.a + cur_test_case.b;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.a += cur_test_case.b;
    EXPECT_EQ(cur_test_case.a, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    auto ret = cur_test_case.a - cur_test_case.b;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.a -= cur_test_case.b;
    EXPECT_EQ(cur_test_case.a, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    auto ret = cur_test_case.a * cur_test_case.b;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.a *= cur_test_case.b;
    EXPECT_EQ(cur_test_case.a, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    auto ret = cur_test_case.a * cur_test_case.b;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.a *= cur_test_case.b;
    EXPECT_EQ(cur_test_case.a, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    auto ret = cur_test_case.a / cur_test_case.b;
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;

    cur_test_case.a /= cur_test_case.b;
    EXPECT_EQ(cur_test_case.a, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Complex<>::Sqrt(cur_test_case.value);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    auto ret = Complex<>::Pow(cur_test_case.value, cur_test_case.n);
    EXPECT_EQ(ret, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    std::stringstream ss;
    ss << cur_test_case.value;
    EXPECT_STREQ(ss.str().c_str(), cur_test_case.want_result.c_str())
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    GetComplex(cur_test_case.len, cur_test_case.in_vec.data(), cur_test_case.out_vec.data());
    EXPECT_EQ(cur_test_case.out_vec, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    ConjugateComplex(cur_test_case.len, cur_test_case.in_vec.data());
    EXPECT_EQ(cur_test_case.in_vec, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    AbsComplex(cur_test_case.len, cur_test_case.in_vec.data(), cur_test_case.out_vec.data());
    EXPECT_EQ(cur_test_case.out_vec, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
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
    TestCase& cur_test_case = test_cases[ii];
    std::stringstream err_info;
    err_info << "Test " << cur_test_case.name << " failed, index " << ii << "\n"
             << "in_vec : " << Vec2Str(cur_test_case.in_vec) << "\n"
             << "want_result : " << Vec2Str(cur_test_case.want_result);

    FFT(cur_test_case.N, cur_test_case.in_vec.data());
    for (uint32_t jj = 0; jj < cur_test_case.N; ++jj) {
      EXPECT_NEAR(cur_test_case.in_vec[jj].real, cur_test_case.want_result[jj].real, 1e-6)
          << err_info.str();
      EXPECT_NEAR(cur_test_case.in_vec[jj].imag, cur_test_case.want_result[jj].imag, 1e-6)
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
    TestCase& cur_test_case = test_cases[ii];
    std::stringstream err_info;
    err_info << "Test " << cur_test_case.name << " failed, index " << ii << "\n"
             << "in_vec : " << Vec2Str(cur_test_case.in_vec) << "\n"
             << "want_result : " << Vec2Str(cur_test_case.want_result);

    IFFT(cur_test_case.N, cur_test_case.in_vec.data());
    for (uint32_t jj = 0; jj < cur_test_case.N; ++jj) {
      EXPECT_NEAR(cur_test_case.in_vec[jj].real, cur_test_case.want_result[jj].real, 1e-6)
          << err_info.str();
      EXPECT_NEAR(cur_test_case.in_vec[jj].imag, cur_test_case.want_result[jj].imag, 1e-6)
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
    TestCase& cur_test_case = test_cases[ii];
    FFTShift(cur_test_case.len, cur_test_case.in_vec.data());
    EXPECT_EQ(cur_test_case.in_vec, cur_test_case.want_result)
        << "Test " << cur_test_case.name << " failed, index " << ii;
  }
}

}  // namespace ytlib
