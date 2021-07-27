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

TEST(MATH_TEST, MATH_TOOL_TEST) {
  cout << Gcd(42, 30) << endl;
  cout << Gcd(770, 26) << endl;
  cout << Gcd(121, 132) << endl;

  std::vector<uint64_t> v;
  Factoring(60, v);
  for (uint32_t ii = 0; ii < v.size(); ++ii) {
    cout << v[ii] << " ";
  }
  cout << endl;
  std::map<uint64_t, uint64_t> m;
  Factoring(60, m);
  for (std::map<uint64_t, uint64_t>::iterator ii = m.begin(); ii != m.end(); ++ii) {
    cout << ii->first << ":" << ii->second << " ";
  }
  cout << endl;

  cout << Mul(5) << endl;     //120
  cout << Mul(9, 5) << endl;  //15120
  cout << Arn(9, 2) << endl;  //72
  cout << Crn(9, 2) << endl;  //36

  cout << SumAP(1.0, 2.0, 3) << endl;   //9
  cout << SumGP(2.0, 1.0, 10) << endl;  //20
  cout << SumGP(2.0, 3.0, 4) << endl;   //80
}

TEST(MATH_TEST, SORT_ALGS_TEST) {
  const uint32_t num = 10;
  int answer[num] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  //冒泡
  int data1[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  bubbleSort(data1, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data1[ii], answer[ii]);
  }

  //归并
  int data2[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  mergeSort(data2, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data2[ii], answer[ii]);
  }

  //归并，非递归
  int data2_2[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  mergeSort2(data2_2, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data2_2[ii], answer[ii]);
  }

  //快排
  int data3[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  quickSort(data3, num);
  for (uint32_t ii = 0; ii < num; ++ii) {
    ASSERT_EQ(data3[ii], answer[ii]);
  }

  //二分查找
  ASSERT_EQ(binarySearch(answer, num, 6), 6);
  ASSERT_EQ(binarySearch(answer, num, -1), num);

  int data4[num] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4};
  ASSERT_EQ(binarySearch(data4, num, 2), 4);
  ASSERT_EQ(binarySearch(data4, num, 1), 2);

  int data5[num] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int data6[num] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int data7[num] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

  ASSERT_EQ(binarySearch(data5, num, 1), num);
  ASSERT_EQ(binarySearch(data6, num, 1), 0);
  ASSERT_EQ(binarySearch(data7, num, 1), num);

  ASSERT_EQ(binarySearchLast(data5, num, 1), num);
  ASSERT_EQ(binarySearchLast(data6, num, 1), 9);
  ASSERT_EQ(binarySearchLast(data7, num, 1), num);
}

}  // namespace ytlib
