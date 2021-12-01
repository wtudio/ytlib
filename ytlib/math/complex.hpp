/**
 * @file complex.hpp
 * @brief 复数
 * @note 复数和相关运算，仅学习用。stl中有复数库<complex>
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <cmath>
#include <concepts>
#include <iostream>

#include "ytlib/math/math_def.h"

namespace ytlib {

/**
 * @brief 简易复数类
 *
 * @tparam CFloat 使用的浮点类型
 */
template <std::floating_point CFloat = double>
class Complex {
 public:
  Complex() {}
  Complex(const CFloat &a, const CFloat &b) : real(a), imag(b) {}
  Complex(const Complex &value) : real(value.real), imag(value.imag) {}
  ~Complex() {}

  bool operator==(const Complex &value) const {
    return (std::abs(this->real - value.real) < 1e-6 && std::abs(this->imag - value.imag) < 1e-6);
  }
  bool operator!=(const Complex &value) const {
    return !(*this == value);
  }

  Complex operator+(const Complex &value) const {
    return Complex(this->real + value.real, this->imag + value.imag);
  }
  Complex &operator+=(const Complex &value) {
    this->real += value.real;
    this->imag += value.imag;
    return *this;
  }

  Complex operator-(const Complex &value) const {
    return Complex(this->real - value.real, this->imag - value.imag);
  }
  Complex &operator-=(const Complex &value) {
    this->real -= value.real;
    this->imag -= value.imag;
    return *this;
  }

  Complex operator*(const Complex &value) const {
    return Complex(
        this->real * value.real - this->imag * value.imag,
        this->real * value.imag + this->imag * value.real);
  }
  Complex &operator*=(const Complex &value) {
    const CFloat &tmp = this->real * value.real - this->imag * value.imag;
    this->imag = this->real * value.imag + this->imag * value.real;
    this->real = tmp;
    return *this;
  }

  Complex operator*(const CFloat &s) const {
    return Complex(this->real * s, this->imag * s);
  }
  Complex &operator*=(const CFloat &s) {
    this->real *= s;
    this->imag *= s;
    return *this;
  }

  Complex operator/(const Complex &value) const {
    const CFloat &tmp_value = value.real * value.real + value.imag * value.imag;
    return Complex(
        (this->real * value.real + this->imag * value.imag) / tmp_value,
        (this->imag * value.real - this->real * value.imag) / tmp_value);
  }
  Complex &operator/=(const Complex &value) {
    const CFloat &tmp_value = value.real * value.real + value.imag * value.imag;
    const CFloat &tmp = (this->real * value.real + this->imag * value.imag) / tmp_value;
    this->imag = (this->imag * value.real - this->real * value.imag) / tmp_value;
    this->real = tmp;
    return *this;
  }

  Complex operator/(const CFloat &s) const {
    return Complex(this->real / s, this->imag / s);
  }
  Complex &operator/=(const CFloat &s) {
    this->real /= s;
    this->imag /= s;
    return *this;
  }

  Complex operator-() const {
    return Complex(-(this->real), -(this->imag));
  }

  void AssignWithExpForm(const CFloat &r, const CFloat &theta) {
    real = r * std::cos(theta);
    imag = r * std::sin(theta);
  }
  CFloat Len() const {
    if (real == 0) return std::abs(imag);
    if (imag == 0) return std::abs(real);
    return std::sqrt(real * real + imag * imag);
  }
  CFloat Angle() const {
    return std::atan2(imag, real);
  }

  static Complex GenWithExpForm(const CFloat &r, const CFloat &theta) {
    return Complex(r * std::cos(theta), r * std::sin(theta));
  }

  static Complex Conj(const Complex &value) {
    return Complex(value.real, -value.imag);
  }

  static Complex Sqrt(const Complex &value) {
    const CFloat &a = std::sqrt(value.Len());
    const CFloat &t = value.Angle() / 2;
    return Complex(a * std::cos(t), a * std::sin(t));
  }

  static Complex Pow(const Complex &value, uint32_t n) {
    Complex re(1.0, 0.0), tmp = value;
    for (; n; n >>= 1) {
      if (n & 1)
        re *= tmp;
      tmp *= tmp;
    }
    return re;
  }

  friend std::ostream &operator<<(std::ostream &output, const Complex &rhs) {
    output << rhs.real;
    const std::string &tmp = std::to_string(rhs.imag);
    if (tmp[0] != '-') {
      output << "+";
    }
    output << rhs.imag << "i";
    return output;
  }

 public:
  CFloat real = 0.0;
  CFloat imag = 0.0;
};

/**
 * @brief 将实数数组扩展成复数数组
 *
 * @tparam CFloat
 * @param[in] len
 * @param[in] in
 * @param[out] out
 */
template <std::floating_point CFloat = double>
void GetComplex(uint32_t len, CFloat in[], Complex<CFloat> out[]) {
  for (uint32_t i = 0; i < len; ++i) {
    out[i].real = in[i];
    out[i].imag = 0;
  }
}

/**
 * @brief 取共轭
 *
 * @tparam CFloat
 * @param[in] len
 * @param[inout] in
 */
template <std::floating_point CFloat = double>
void ConjugateComplex(uint32_t len, Complex<CFloat> in[]) {
  for (uint32_t i = 0; i < len; ++i) {
    in[i].imag = -in[i].imag;
  }
}

/**
 * @brief 复数数组取模
 *
 * @tparam CFloat
 * @param[in] len
 * @param[in] in
 * @param[out] out
 */
template <std::floating_point CFloat = double>
void AbsComplex(uint32_t len, Complex<CFloat> in[], CFloat out[]) {
  for (uint32_t i = 0; i < len; ++i) {
    out[i] = in[i].Len();
  }
}

/**
 * @brief 傅立叶变换
 *
 * @tparam CFloat
 * @param[in] N
 * @param[inout] data
 */
template <std::floating_point CFloat = double>
void FFT(uint32_t N, Complex<CFloat> data[]) {
  uint32_t k, M = 1;
  // 计算分解的级数M=log2(N)
  for (uint32_t i = N; (i >>= 1) != 1; ++M) {
  }

  // 按照倒位序重新排列原信号
  for (uint32_t i = 1, j = N >> 1; i <= N - 2; ++i) {
    if (i < j) {
      std::swap(data[j], data[i]);
    }
    k = N >> 1;
    while (k <= j) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }

  // fft
  uint32_t r, la, lb, lc;
  for (uint32_t m = 1; m <= M; ++m) {
    la = 1 << m;
    // la=2^m代表第m级每个分组所含节点数
    lb = la >> 1;  // lb代表第m级每个分组所含碟形单元数,同时它也表示每个碟形单元上下节点之间的距离
    // 碟形运算
    for (uint32_t l = 0; l < lb; ++l) {
      r = l << (M - m);
      Complex<CFloat> tmp(cos(2 * MATH_PI * r / N), -sin(2 * MATH_PI * r / N));
      // 遍历每个分组，分组总数为N/la
      for (uint32_t n = l; n < N - 1; n += la) {
        lc = n + lb;  // n,lc分别代表一个碟形单元的上、下节点编号
        Complex<CFloat> t(data[lc] * tmp);
        data[lc] = data[n] - t;
        data[n] += t;
      }
    }
  }
}

/**
 * @brief 傅里叶逆变换
 *
 * @tparam CFloat
 * @param[in] N
 * @param[inout] data
 */
template <std::floating_point CFloat = double>
void IFFT(uint32_t N, Complex<CFloat> data[]) {
  ConjugateComplex(N, data);
  FFT(N, data);
  ConjugateComplex(N, data);
  for (uint32_t i = 0; i < N; ++i) {
    data[i] /= N;
  }
}

/**
 * @brief fftshift
 *
 * @tparam CFloat
 * @param[in] len
 * @param[inout] data
 */
template <std::floating_point CFloat = double>
void FFTShift(uint32_t len, Complex<CFloat> data[]) {
  len /= 2;
  for (uint32_t i = 0; i < len; ++i) {
    std::swap(data[i + len], data[i]);
  }
}

template <std::floating_point CFloat = double>
CFloat abs(const Complex<CFloat> &value) { return value.Len(); }

template <std::floating_point CFloat = double>
Complex<CFloat> sqrt(const Complex<CFloat> &value) { return Complex<CFloat>::Sqrt(value); }

}  // namespace ytlib
