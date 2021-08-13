#include <gtest/gtest.h>

#include "big_num.hpp"
#include "complex.hpp"
#include "math_util.hpp"
#include "matrix.hpp"
#include "sort_algs.hpp"

namespace ytlib {

using std::cout;
using std::endl;
using std::vector;

TEST(MATH_TEST, Complex_TEST) {
  Complex c;
  Complex c1(0.5, 6.3);
  Complex c2(c1);

  c = c1 + c2;

  Complex c3 = c;
  cout << c3 << endl;

  Complex c4(c3 += c);
  c.swap(c4);

  Complex c5 = Complex<double>::conj(c4);

  double a = Complex<double>::abs(c5);
  double b = Complex<double>::angle(c5);

  Complex c6 = Complex<double>::sqrt(c5);

  int32_t count = 16;
  Complex<double> *cc = new Complex<double>[count];
  double *ff = new double[count];

  for (int32_t ii = 0; ii < count; ++ii) {
    ff[ii] = std::pow(ii, 1.7);
    cc[ii].real = std::sqrt(ii * 3);
    cc[ii].imag = std::pow(ii, 1.5);
    cout << cc[ii] << endl;
  }
  ConjugateComplex(count, cc);
  for (int32_t ii = 0; ii < count; ++ii) {
    cout << cc[ii] << endl;
  }

  Complex<double> *cc0 = new Complex<double>[count];
  GetComplex(count, ff, cc0);
  for (int32_t ii = 0; ii < count; ++ii) {
    cout << cc0[ii] << endl;
  }

  FFT(count, cc);
  for (int32_t ii = 0; ii < count; ++ii) {
    cout << cc[ii] << endl;
  }
  IFFT(count, cc);
  for (int32_t ii = 0; ii < count; ++ii) {
    cout << cc[ii] << endl;
  }
  FFTShift(count, cc);
  for (int32_t ii = 0; ii < count; ++ii) {
    cout << cc[ii] << endl;
  }

  delete[] cc;
  delete[] cc0;
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

TEST(MATH_UTIL_TEST, Gcd_test) {
  struct TestCaseForGcd {
    std::string name;

    uint64_t num1;
    uint64_t num2;

    uint64_t want_result;
  };
  std::vector<TestCaseForGcd> test_cases;

  test_cases.emplace_back(TestCaseForGcd{
      .name = "good case 1",
      .num1 = 42,
      .num2 = 30,
      .want_result = 6});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "good case 2",
      .num1 = 770,
      .num2 = 26,
      .want_result = 2});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "good case 3",
      .num1 = 121,
      .num2 = 132,
      .want_result = 11});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 1",
      .num1 = 1,
      .num2 = 1,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 2",
      .num1 = 1,
      .num2 = 2,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 3",
      .num1 = 1,
      .num2 = 0,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForGcd{
      .name = "bad case 4",
      .num1 = 0,
      .num2 = 0,
      .want_result = 1});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(Gcd(test_cases[ii].num1, test_cases[ii].num2), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, Lcm_test) {
  struct TestCaseForLcm {
    std::string name;

    uint64_t num1;
    uint64_t num2;

    uint64_t want_result;
  };
  std::vector<TestCaseForLcm> test_cases;

  test_cases.emplace_back(TestCaseForLcm{
      .name = "good case 1",
      .num1 = 1,
      .num2 = 10,
      .want_result = 10});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "good case 2",
      .num1 = 2,
      .num2 = 5,
      .want_result = 10});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "good case 3",
      .num1 = 4,
      .num2 = 10,
      .want_result = 20});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "bad case 1",
      .num1 = 1,
      .num2 = 1,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "bad case 2",
      .num1 = 0,
      .num2 = 100,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForLcm{
      .name = "bad case 3",
      .num1 = 0,
      .num2 = 0,
      .want_result = 0});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(Lcm(test_cases[ii].num1, test_cases[ii].num2), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, IsPrime_test) {
  struct TestCaseForIsPrime {
    std::string name;

    uint64_t num;

    bool want_result;
  };
  std::vector<TestCaseForIsPrime> test_cases;

  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 1",
      .num = 0,
      .want_result = false});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 2",
      .num = 1,
      .want_result = false});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 3",
      .num = 2,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 4",
      .num = 3,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 5",
      .num = 4,
      .want_result = false});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 6",
      .num = 23,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 7",
      .num = 997,
      .want_result = true});
  test_cases.emplace_back(TestCaseForIsPrime{
      .name = "case 8",
      .num = 997 * 991,
      .want_result = false});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(IsPrime(test_cases[ii].num), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, CountOne_test) {
  struct TestCaseForCountOne {
    std::string name;

    uint64_t n;

    uint8_t want_result;
  };
  std::vector<TestCaseForCountOne> test_cases;
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 1",
      .n = 0b0,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 2",
      .n = 0b1000,
      .want_result = 1});
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 3",
      .n = 0b1001,
      .want_result = 2});
  test_cases.emplace_back(TestCaseForCountOne{
      .name = "case 4",
      .n = 0b1010010001,
      .want_result = 4});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    EXPECT_EQ(CountOne(test_cases[ii].n), test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
    EXPECT_EQ(CountZero(test_cases[ii].n), 64 - test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_UTIL_TEST, FindFirstOne_test) {
  struct TestCaseForFindFirstOne {
    std::string name;

    uint64_t val;
    uint8_t pos;

    uint8_t want_result;
  };
  std::vector<TestCaseForFindFirstOne> test_cases;
  test_cases.emplace_back(TestCaseForFindFirstOne{
      .name = "case 1",
      .val = 0b101000,
      .pos = 0,
      .want_result = 3});
  test_cases.emplace_back(TestCaseForFindFirstOne{
      .name = "case 1",
      .val = 0b101001,
      .pos = 0,
      .want_result = 0});
  test_cases.emplace_back(TestCaseForFindFirstOne{
      .name = "case 1",
      .val = 0b101001,
      .pos = 1,
      .want_result = 3});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    auto ret = FindFirstOne(test_cases[ii].val, test_cases[ii].pos);
    EXPECT_EQ(ret, test_cases[ii].want_result)
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(MATH_TEST, MATH_UTIL_TEST) {
  std::vector<uint64_t> v = Factoring2Vec(60);
  std::vector<uint64_t> tgt_v{2, 2, 3, 5};
  ASSERT_EQ(v.size(), tgt_v.size());
  for (uint32_t ii = 0; ii < v.size(); ++ii) {
    ASSERT_EQ(v[ii], tgt_v[ii]);
  }

  std::map<uint64_t, uint64_t> m = Factoring2Map(60);
  std::map<uint64_t, uint64_t> tgt_m{{2, 2}, {3, 1}, {5, 1}};
  for (const auto &itr : m) {
    auto tgt_itr = tgt_m.find(itr.first);
    ASSERT_TRUE(tgt_itr != tgt_m.end());
    ASSERT_EQ(itr.second, tgt_itr->second);
  }

  ASSERT_EQ(Mul(0), 0);
  ASSERT_EQ(Mul(5), 120);
  ASSERT_EQ(Mul(9, 5), 15120);
  ASSERT_EQ(Mul(9, 9), 9);
  ASSERT_EQ(Mul(9, 10), 0);

  ASSERT_EQ(Arn(9, 2), 72);
  ASSERT_EQ(Arn(0, 2), 0);
  ASSERT_EQ(Arn(2, 0), 0);
  ASSERT_EQ(Arn(9, 1), 9);
  ASSERT_EQ(Arn(4, 4), 24);

  ASSERT_EQ(Arn(9, 2, Arn(9, 1), 1), 72);
  ASSERT_EQ(Arn(9, 2, Arn(9, 2), 2), 72);
  ASSERT_EQ(Arn(9, 2, Arn(9, 3), 3), 72);

  ASSERT_EQ(Crn(9, 2), 36);
  ASSERT_EQ(Crn(0, 2), 0);
  ASSERT_EQ(Crn(2, 0), 0);
  ASSERT_EQ(Crn(9, 1), 9);
  ASSERT_EQ(Crn(4, 4), 1);

  ASSERT_EQ(Crn(9, 2, Crn(9, 1), 1), 36);
  ASSERT_EQ(Crn(9, 2, Crn(9, 2), 2), 36);
  ASSERT_EQ(Crn(9, 2, Crn(9, 3), 3), 36);

  ASSERT_TRUE(std::abs(SumAP(1.0, 2.0, 3) - 9.0) < 1e-6);

  ASSERT_TRUE(std::abs(SumGP(2.0, 0.0, 10)) < 1e-6);
  ASSERT_TRUE(std::abs(SumGP(0.0, 2.0, 10)) < 1e-6);
  ASSERT_TRUE(std::abs(SumGP(0.0, 0.0, 10)) < 1e-6);
  ASSERT_TRUE(std::abs(SumGP(2.0, 1.0, 10) - 20.0) < 1e-6);
  ASSERT_TRUE(std::abs(SumGP(2.0, 3.0, 4) - 80.0) < 1e-6);
  ASSERT_TRUE(std::abs(SumGP(2.0, 3.0, 0)) < 1e-6);

  ASSERT_TRUE(std::abs(Pow(0.0, 1)) < 1e-6);
  ASSERT_TRUE(std::abs(Pow(1.0, 0) - 1.0) < 1e-6);
  ASSERT_TRUE(std::abs(Pow(2.0, 1) - 2.0) < 1e-6);
  ASSERT_TRUE(std::abs(Pow(2.0, 10) - 1024.0) < 1e-6);
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
