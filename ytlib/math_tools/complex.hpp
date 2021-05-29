/**
 * @file complex.hpp
 * @brief 复数
 * @details 复数和相关运算，仅学习用。stl中有复数库<complex>
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <concepts>
#include <iostream>

#include "math_def.hpp"

namespace ytlib {

/**
 * @brief 简易复数类
 */
template <std::floating_point Float = double>
class Complex {
 public:
  Complex() : real(0.0), imag(0.0) {}
  Complex(const Float &a, const Float &b) : real(a), imag(b) {}
  Complex(const Complex &value) : real(value.real), imag(value.imag) {}
  ~Complex() {}

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
    const Float &tmp = this->real * value.real - this->imag * value.imag;
    this->imag = this->real * value.imag + this->imag * value.real;
    this->real = tmp;
    return *this;
  }

  Complex operator*(const Float &s) const {
    return Complex(this->real * s, this->imag * s);
  }
  Complex &operator*=(const Float &s) {
    this->real *= s;
    this->imag *= s;
    return *this;
  }

  Complex operator/(const Complex &value) const {
    const Float &abs_value = Complex::abs(value);
    return Complex(
        (this->real * value.real + this->imag * value.imag) / abs_value,
        (this->real * value.imag - this->imag * value.real) / abs_value);
  }
  Complex &operator/=(const Complex &value) {
    const Float &abs_value = Complex::abs(value);
    const Float &tmp = (this->real * value.real + this->imag * value.imag) / abs_value;
    this->imag = (this->real * value.imag - this->imag * value.real) / abs_value;
    this->real = tmp;
    return *this;
  }

  Complex operator/(const Float &s) const {
    return Complex(this->real / s, this->imag / s);
  }
  Complex &operator/=(const Float &s) {
    this->real /= s;
    this->imag /= s;
    return *this;
  }

  Complex operator-() const {
    return Complex(-(this->real), -(this->imag));
  }

  Complex &swap(Complex &value) {
    if (this != &value) {
      Float tmp = this->real;
      this->real = value.real;
      value.real = tmp;
      tmp = this->imag;
      this->imag = value.imag;
      value.imag = tmp;
    }
    return *this;
  }

  static Complex conj(const Complex &value) {
    return Complex(value.real, -value.imag);
  }
  static Float abs(const Complex &value) {
    return std::sqrt(value.real * value.real + value.imag * value.imag);
  }
  static Float angle(const Complex &value) {
    return std::atan2(value.imag, value.real);
  }

  static Complex sqrt(const Complex &value) {
    const Float &a = std::sqrt(abs(value));
    const Float &t = angle(value) / 2;
    return Complex(a * std::cos(t), a * std::sin(t));
  }

  static Complex pow(const Complex &value, uint32_t n) {
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
    std::string tmp(std::to_string(rhs.imag));
    if (tmp[0] != '-') {
      output << "+";
    }
    output << tmp << "i";
    return output;
  }

  // direct data access
  Float real;
  Float imag;
};

///将实数数组扩展成复数数组
template <std::floating_point Float = double>
void GetComplex(uint32_t len, Float in[], Complex<Float> out[]) {
  for (uint32_t i = 0; i < len; ++i) {
    out[i].real = in[i];
    out[i].imag = 0;
  }
}

///取共轭
template <std::floating_point Float = double>
void ConjugateComplex(uint32_t len, Complex<Float> in[]) {
  for (uint32_t i = 0; i < len; ++i) {
    in[i].imag = -in[i].imag;
  }
}

///复数数组取模
template <std::floating_point Float = double>
void AbsComplex(uint32_t len, Complex<Float> f[], Float out[]) {
  for (uint32_t i = 0; i < len; ++i) {
    out[i] = Complex<Float>::abs(f[i]);
  }
}

///傅立叶变换 输出也存在数组f中
template <std::floating_point Float = double>
void FFT(uint32_t N, Complex<Float> f[]) {
  uint32_t k, M = 1;
  /*----计算分解的级数M=log2(N)----*/
  for (uint32_t i = N; (i >>= 1) != 1; ++M) {
  }

  /*----按照倒位序重新排列原信号----*/
  for (uint32_t i = 1, j = N >> 1; i <= N - 2; ++i) {
    if (i < j) {
      f[j].swap(f[i]);
    }
    k = N >> 1;
    while (k <= j) {
      j -= k;
      k >>= 1;
    }
    j += k;
  }
  /*----FFT算法----*/
  uint32_t r, la, lb, lc;
  for (uint32_t m = 1; m <= M; ++m) {
    la = 1 << m;
    //la=2^m代表第m级每个分组所含节点数
    lb = la >> 1;  //lb代表第m级每个分组所含碟形单元数,同时它也表示每个碟形单元上下节点之间的距离
    //----碟形运算----
    for (uint32_t l = 0; l < lb; ++l) {
      r = l << (M - m);
      Complex<Float> tmp(cos(2 * MATH_PI * r / N), -sin(2 * MATH_PI * r / N));
      //遍历每个分组，分组总数为N/la
      for (uint32_t n = l; n < N - 1; n += la) {
        lc = n + lb;  //n,lc分别代表一个碟形单元的上、下节点编号
        Complex<Float> t(f[lc] * tmp);
        f[lc] = f[n] - t;
        f[n] += t;
      }
    }
  }
}

///傅里叶逆变换
template <std::floating_point Float = double>
void IFFT(uint32_t N, Complex<Float> f[]) {
  ConjugateComplex(N, f);
  FFT(N, f);
  ConjugateComplex(N, f);
  for (uint32_t i = 0; i < N; ++i) {
    f[i] /= N;
  }
}

/// fftshift
template <std::floating_point Float = double>
void FFTShift(uint32_t len, Complex<Float> f[]) {
  len /= 2;
  for (uint32_t i = 0; i < len; ++i) {
    f[i + len].swap(f[i]);
  }
}

template <std::floating_point Float = double>
Float abs(const Complex<Float> &value) { return Complex<Float>::abs(value); }

template <std::floating_point Float = double>
Complex<Float> sqrt(const Complex<Float> &value) { return Complex<Float>::sqrt(value); }

template <std::floating_point Float = double>
void swap(Complex<Float> &a, Complex<Float> &b) { a.swap(b); }
}  // namespace ytlib
