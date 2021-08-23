#include <gtest/gtest.h>

#include "big_num.hpp"
#include "complex.hpp"
#include "matrix.hpp"
#include "sort_algs.hpp"
#include "ytlib/misc/stl_util.hpp"

namespace ytlib {

using std::cout;
using std::endl;
using std::vector;

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

  c4.Swap(c3);
  EXPECT_DOUBLE_EQ(c4.real, 3.0);
  EXPECT_DOUBLE_EQ(c4.imag, 4.0);
  EXPECT_DOUBLE_EQ(c3.real, -3.0);
  EXPECT_DOUBLE_EQ(c3.imag, -4.0);

  swap(c4, c3);
  EXPECT_DOUBLE_EQ(c4.real, -3.0);
  EXPECT_DOUBLE_EQ(c4.imag, -4.0);
  EXPECT_DOUBLE_EQ(c3.real, 3.0);
  EXPECT_DOUBLE_EQ(c3.imag, 4.0);

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
    EXPECT_STREQ(Vec2Str(test_cases[ii].out_vec).c_str(), Vec2Str(test_cases[ii].want_result).c_str())
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
    EXPECT_STREQ(Vec2Str(test_cases[ii].in_vec).c_str(), Vec2Str(test_cases[ii].want_result).c_str())
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
    EXPECT_STREQ(Vec2Str(test_cases[ii].out_vec).c_str(), Vec2Str(test_cases[ii].want_result).c_str())
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
    test_case.in_vec.resize(test_case.N);

    double A = 2.0;
    uint32_t k = 2;
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
    test_case.in_vec.resize(test_case.N);

    double A = 5.0;
    uint32_t k = 4;
    for (uint32_t ii = 0; ii < test_case.N; ++ii) {
      test_case.in_vec[ii].AssignWithExpForm(A, k * ii * MATH_PI / test_case.N);
    }

    test_case.want_result.resize(test_case.N);
    test_case.want_result[k / 2] = Complex<double>(A * test_case.N, 0.0);

    test_cases.emplace_back(std::move(test_case));
  }

  {
    TestCase test_case{
        .name = "case 4",
        .N = 16};
    test_case.in_vec.resize(test_case.N);

    double A = 10.0;
    uint32_t k = 12;
    for (uint32_t ii = 0; ii < test_case.N; ++ii) {
      test_case.in_vec[ii].AssignWithExpForm(A, k * ii * MATH_PI / test_case.N);
    }

    test_case.want_result.resize(test_case.N);
    test_case.want_result[k / 2] = Complex<double>(A * test_case.N, 0.0);

    test_cases.emplace_back(std::move(test_case));
  }

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    FFT(test_cases[ii].N, test_cases[ii].in_vec.data());
    for (uint32_t jj = 0; jj < test_cases[ii].N; ++jj) {
      EXPECT_NEAR(test_cases[ii].in_vec[jj].real, test_cases[ii].want_result[jj].real, 1e-6)
          << "Test " << test_cases[ii].name << " failed, index " << ii << "\n"
          << "in_vec : " << Vec2Str(test_cases[ii].in_vec) << "\n"
          << "want_result : " << Vec2Str(test_cases[ii].want_result);
      EXPECT_NEAR(test_cases[ii].in_vec[jj].imag, test_cases[ii].want_result[jj].imag, 1e-6)
          << "Test " << test_cases[ii].name << " failed, index " << ii << "\n"
          << "in_vec : " << Vec2Str(test_cases[ii].in_vec) << "\n"
          << "want_result : " << Vec2Str(test_cases[ii].want_result);
    }
  }
}

TEST(MATH_TEST, Matrix_TEST) {
  int32_t count = 9;
  Matrix m;
  cout << m << endl;
  Matrix m1(3, 2);
  cout << m1 << endl;
  double *f = new double[count];
  for (int32_t ii = 0; ii < count; ++ii) {
    f[ii] = ii * ii;
  }

  Matrix m2(3, 3, f);
  cout << m2 << endl;
  Matrix m3(m2);
  cout << m3 << endl;

  m = m3;
  cout << m << endl;

  double *f2 = new double[count];
  m3.getData(f2, 1, 1, 1, 2);
  for (int32_t ii = 0; ii < count; ++ii) {
    cout << f2[ii] << endl;
  }

  Matrix m4 = m3.getMat(0, 0);
  cout << m4 << endl;

  Matrix m5(5, 6);
  m5.setMat(m4);
  cout << m5 << endl;
  m5.setMat(m4, 2, 2);
  cout << m5 << endl;

  m5.setVal(55.5, 4, 4);
  cout << m5 << endl;

  m5.setDiag(77.5);
  cout << m5 << endl;

  cout << m << endl;
  std::swap(m5, m);
  cout << m5 << endl;
  cout << m << endl;

  int32_t ii[3] = {1, 2, 3};
  Matrix m6 = m.extractCols(ii, 3);
  cout << m6 << endl;

  m5.zero();
  cout << m5 << endl;

  Matrix m7 = Matrix::eye(5);
  cout << m7 << endl;

  m.eye();
  cout << m << endl;

  Matrix m8 = Matrix::ones(6, 7);
  cout << m8 << endl;

  Matrix m9 = Matrix::reshape(m, 6, 5);
  cout << m9 << endl;

  cout << Matrix::rotMatX(0.5) << endl;
  cout << Matrix::rotMatY(0.5) << endl;
  cout << Matrix::rotMatZ(0.5) << endl;

  Matrix m10 = Matrix::ones(4, 4);
  m10 = Matrix::pow(m10, 3);
  cout << m10 << endl;

  Matrix m11 = Matrix::ones(4, 4);
  cout << m11 << endl;

  cout << m11 + m10 << endl;
  m11 += m10;
  cout << m11 << endl;

  cout << m11 - m10 << endl;
  m11 -= m10;
  cout << m11 << endl;

  cout << m11 * 3 << endl;
  m11 *= 3;
  cout << m11 << endl;

  cout << m11 / 3 << endl;
  m11 /= 3;
  cout << m11 << endl;

  cout << -m11 << endl;

  Matrix m12 = Matrix::ones(2, 3);
  cout << m12 << endl;
  Matrix m13 = Matrix::ones(3, 4);
  cout << m13 << endl;

  cout << m12 * m13 << endl;

  m12 *= m13;
  cout << m12 << endl;

  cout << ~m12 << endl;

  delete[] f;
  delete[] f2;
}

TEST(MATH_TEST, BigNum_TEST) {
  BigNum a((int64_t(1) << 32) + 1);
  cout << a << endl;
  BigNum b((int64_t(1) << 32) + 1, 10);
  cout << b << endl;

  BigNum c("123456789ABCDEF");
  cout << c << endl;

  BigNum d("123456789123456789", 10);
  cout << d << endl;

  BigNum e("-123456789123456789", 10);
  cout << e << endl;

  BigNum f = e + d;
  cout << f << endl;

  BigNum g("9999", 10);
  BigNum h("111", 10);
  cout << g + h << endl;

  BigNum g1("10000", 10);
  BigNum h1("-111", 10);
  cout << g1 + h1 << endl;

  cout << g * h << endl;
  g *= h;
  cout << g << endl;
}

TEST(MATH_TEST, SORT_ALGS_TEST) {
  const uint32_t num = 10;
  int answer[num] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  //冒泡
  int data1[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  BubbleSort(data1, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data1[ii], answer[ii]);
  }

  //归并
  int data2[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  MergeSort(data2, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data2[ii], answer[ii]);
  }

  //归并，非递归
  int data2_2[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  MergeSort2(data2_2, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data2_2[ii], answer[ii]);
  }

  //快排
  int data3[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  QuickSort(data3, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data3[ii], answer[ii]);
  }

  //二分查找
  ASSERT_EQ(BinarySearch(answer, num, 6), 6);
  ASSERT_EQ(BinarySearch(answer, num, -1), num);

  int data4[num] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4};
  ASSERT_EQ(BinarySearch(data4, num, 2), 4);
  ASSERT_EQ(BinarySearch(data4, num, 1), 2);

  int data5[num] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int data6[num] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int data7[num] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

  ASSERT_EQ(BinarySearch(data5, num, 1), num);
  ASSERT_EQ(BinarySearch(data6, num, 1), 0);
  ASSERT_EQ(BinarySearch(data7, num, 1), num);

  ASSERT_EQ(BinarySearchLast(data5, num, 1), num);
  ASSERT_EQ(BinarySearchLast(data6, num, 1), 9);
  ASSERT_EQ(BinarySearchLast(data7, num, 1), num);
}

}  // namespace ytlib
