/**
 * @file Complex.h
 * @brief 复数
 * @details 复数和相关运算
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/LightMath/Mathbase.h>
#include <iostream>
#include <sstream>

namespace ytlib {
/**
 * @brief 简易复数类
 */
class Complex {
 public:
  Complex() : real(0.0), imag(0.0) {}
  Complex(const tfloat a, const tfloat b) : real(a), imag(b) {}
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
    tfloat tmp = this->real * value.real - this->imag * value.imag;
    this->imag = this->real * value.imag + this->imag * value.real;
    this->real = tmp;
    return *this;
  }

  Complex operator*(const tfloat &s) const {
    return Complex(this->real * s, this->imag * s);
  }
  Complex &operator*=(const tfloat &s) {
    this->real *= s;
    this->imag *= s;
    return *this;
  }

  Complex operator/(const Complex &value) const {
    tfloat abs_value = Complex::abs(value);
    return Complex(
        (this->real * value.real + this->imag * value.imag) / abs_value,
        (this->real * value.imag - this->imag * value.real) / abs_value);
  }
  Complex &operator/=(const Complex &value) {
    tfloat abs_value = Complex::abs(value);
    tfloat tmp = (this->real * value.real + this->imag * value.imag) / abs_value;
    this->imag = (this->real * value.imag - this->imag * value.real) / abs_value;
    this->real = tmp;
    return *this;
  }

  Complex operator/(const tfloat &s) const {
    return Complex(this->real / s, this->imag / s);
  }
  Complex &operator/=(const tfloat &s) {
    this->real /= s;
    this->imag /= s;
    return *this;
  }

  Complex operator-() const {
    return Complex(-(this->real), -(this->imag));
  }

  Complex &swap(Complex &value) {
    if (this != &value) {
      tfloat tmp = this->real;
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
  static tfloat abs(const Complex &value) {
    return std::sqrt(value.real * value.real + value.imag * value.imag);
  }
  static tfloat angle(const Complex &value) {
    return std::atan2(value.imag, value.real);
  }

  static Complex sqrt(const Complex &value) {
    tfloat a = std::sqrt(abs(value));
    tfloat t = angle(value) / 2;
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
    //std::stringstream ss;
    //ss << rhs.imag;
    //std::string tmp(ss.str());
    std::string tmp(std::to_string(rhs.imag));
    if (tmp[0] != '-') {
      output << "+";
    }
    output << tmp << "i";
    return output;
  }

  // direct data access
  tfloat real;
  tfloat imag;
};

///将实数数组扩展成复数数组
inline void get_complex(int32_t n, tfloat in[], Complex out[]) {
  for (int32_t i = 0; i < n; ++i) {
    out[i].real = in[i];
    out[i].imag = 0;
  }
}
///取共轭
inline void conjugate_complex(int32_t n, Complex in[]) {
  for (int32_t i = 0; i < n; ++i) {
    in[i].imag = -in[i].imag;
  }
}
///复数数组取模
inline void c_abs(int32_t n, Complex f[], tfloat out[]) {
  for (int32_t i = 0; i < n; ++i) {
    out[i] = Complex::abs(f[i]);
  }
}
///傅立叶变换 输出也存在数组f中
inline void fft(int32_t N, Complex f[]) {
  int32_t k, M = 1;
  /*----计算分解的级数M=log2(N)----*/
  for (int32_t i = N; (i >>= 1) != 1; ++M)
    ;
  /*----按照倒位序重新排列原信号----*/
  for (int32_t i = 1, j = N >> 1; i <= N - 2; ++i) {
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
  int32_t r, la, lb, lc;
  for (int32_t m = 1; m <= M; ++m) {
    la = 1 << m;
    //la=2^m代表第m级每个分组所含节点数
    lb = la >> 1;  //lb代表第m级每个分组所含碟形单元数,同时它也表示每个碟形单元上下节点之间的距离
    //----碟形运算----
    for (int32_t l = 0; l < lb; ++l) {
      r = l << (M - m);
      Complex tmp(cos(2 * PI * r / N), -sin(2 * PI * r / N));
      //遍历每个分组，分组总数为N/la
      for (int32_t n = l; n < N - 1; n += la) {
        lc = n + lb;  //n,lc分别代表一个碟形单元的上、下节点编号
        Complex t(f[lc] * tmp);
        f[lc] = f[n] - t;
        f[n] += t;
      }
    }
  }
}
/// 傅里叶逆变换
inline void ifft(int32_t N, Complex f[]) {
  conjugate_complex(N, f);
  fft(N, f);
  conjugate_complex(N, f);
  for (int32_t i = 0; i < N; ++i) {
    f[i] /= N;
  }
}
/// fftshift
inline void fftshift(int32_t len, Complex f[]) {
  len /= 2;
  for (int32_t i = 0; i < len; ++i) {
    f[i + len].swap(f[i]);
  }
}

inline tfloat abs(const Complex &value) { return Complex::abs(value); }
inline Complex sqrt(const Complex &value) { return Complex::sqrt(value); }
inline void swap(Complex &a, Complex &b) { a.swap(b); }
}  // namespace ytlib
