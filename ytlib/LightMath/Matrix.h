/**
 * @file Matrix.h
 * @brief 矩阵类
 * @details 基于模板的简易矩阵类型
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <cstring>
#include <iostream>
#include <string>

#include <ytlib/LightMath/Complex.h>
#include <ytlib/LightMath/Mathbase.h>

#define MAXSIZE 268435456

namespace ytlib {
/**
 * @brief 简易矩阵类
 * 模板形式的简易矩阵类，可以放复数、int、float、double、自定义类型等等
 */
template <typename T>
class Basic_Matrix {
 public:
  // direct data access
  T** val;
  int32_t m, n;  //m行n列。每行的数据是连续的，应尽量使行号为索引

  Basic_Matrix() : m(0), n(0), val(NULL) {}
  Basic_Matrix(const int32_t m_, const int32_t n_) : val(NULL) {
    _allocateMemory(m_, n_);
  }
  Basic_Matrix(const int32_t m_, const int32_t n_, const T* val_, int32_t N_ = 0) : val(NULL) {
    _allocateMemory(m_, n_);
    if (N_ == 0) N_ = m * n;
    int32_t k = 0;
    for (int32_t i = 0; i < m_; ++i)
      for (int32_t j = 0; j < n_; ++j) {
        if (k >= N_) return;
        val[i][j] = val_[k++];
      }
  }
  Basic_Matrix(const Basic_Matrix& M) : val(NULL) {
    _allocateMemory(M.m, M.n);
    memcpy(val[0], M.val[0], M.n * M.m * sizeof(T));
  }
  ~Basic_Matrix() {
    _releaseMemory();
  }
  Basic_Matrix& operator=(const Basic_Matrix& M) {
    if (this != &M) {
      if (M.m != m || M.n != n) {
        _releaseMemory();
        _allocateMemory(M.m, M.n);
      }
      memcpy(val[0], M.val[0], M.n * M.m * sizeof(T));
    }
    return *this;
  }
  //右值引用
  Basic_Matrix(Basic_Matrix&& M) : val(M.val), m(M.m), n(M.n) {
    M.val = NULL;
    M.m = M.n = 0;
  }
  Basic_Matrix& operator=(Basic_Matrix&& M) {
    if (this != &M) {
      //先释放自己的资源
      _releaseMemory();
      val = M.val;
      m = M.m;
      n = M.n;
      M.val = NULL;
      M.m = M.n = 0;
    }
    return *this;
  }

  /// copies submatrix of M into array 'val', default values copy whole row/column/matrix
  void getData(T* val_, int32_t i1 = 0, int32_t j1 = 0, int32_t i2 = -1, int32_t j2 = -1, int32_t N_ = 0) const {
    if (i2 == -1) i2 = m - 1;
    if (j2 == -1) j2 = n - 1;
    assert(i1 >= 0 && i2 < m && j1 >= 0 && j2 < n && i2 >= i1 && j2 >= j1);
    if (N_ == 0) N_ = (i2 - i1 + 1) * (j2 - j1 + 1);
    int32_t k = 0;
    for (int32_t i = i1; i <= i2; ++i)
      for (int32_t j = j1; j <= j2; ++j) {
        if (k >= N_) return;
        val_[k++] = val[i][j];
      }
  }

  /// set or get submatrices of current matrix
  Basic_Matrix getMat(int32_t i1, int32_t j1, int32_t i2 = -1, int32_t j2 = -1) const {
    if (i2 == -1) i2 = m - 1;
    if (j2 == -1) j2 = n - 1;
    assert(i1 >= 0 && i2 < m && j1 >= 0 && j2 < n && i2 >= i1 && j2 >= j1);
    Basic_Matrix M(i2 - i1 + 1, j2 - j1 + 1);
    for (int32_t i = 0; i < M.m; ++i)
      for (int32_t j = 0; j < M.n; ++j)
        M.val[i][j] = val[i1 + i][j1 + j];
    return M;
  }

  void setMat(const Basic_Matrix& M, const int32_t i1 = 0, const int32_t j1 = 0) {
    assert(i1 >= 0 && j1 >= 0 && (i1 + M.m) <= m && (j1 + M.n) <= n);
    for (int32_t i = 0; i < M.m; ++i)
      for (int32_t j = 0; j < M.n; ++j)
        val[i1 + i][j1 + j] = M.val[i][j];
  }

  /// set sub-matrix to scalar (default 0), -1 as end replaces whole row/column/matrix
  void setVal(const T& s, int32_t i1 = 0, int32_t j1 = 0, int32_t i2 = -1, int32_t j2 = -1) {
    if (i2 == -1) i2 = m - 1;
    if (j2 == -1) j2 = n - 1;
    assert(i1 >= 0 && i2 < m && j1 >= 0 && j2 < n && i2 >= i1 && j2 >= j1);
    for (int32_t i = i1; i <= i2; ++i)
      for (int32_t j = j1; j <= j2; ++j)
        val[i][j] = s;
  }

  /// set (part of) diagonal to scalar, -1 as end replaces whole diagonal
  void setDiag(const T& s, int32_t i1 = 0, int32_t i2 = -1) {
    if (i2 == -1) i2 = std::min(m, n) - 1;
    assert(i1 >= 0 && i2 < std::min(m, n) && i2 >= i1);
    for (int32_t i = i1; i <= i2; ++i)
      val[i][i] = s;
  }

  /// clear matrix
  void zero() {
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        val[i][j] = T();
  }
  Basic_Matrix& swap(Basic_Matrix& value) {
    if (this != &value) {
      int32_t tmp = value.m;
      value.m = this->m;
      this->m = tmp;
      tmp = value.n;
      value.n = this->n;
      this->n = tmp;
      T** ptmp = value.val;
      value.val = this->val;
      this->val = ptmp;
    }
    return *this;
  }
  /// extract columns with given index
  Basic_Matrix extractCols(const int32_t* idx, const int32_t N_) const {
    Basic_Matrix M(m, N_);
    for (int32_t j = 0; j < N_; ++j) {
      assert((idx[j] < n) && (idx[j] >= 0));
      for (int32_t i = 0; i < m; ++i)
        M.val[i][j] = val[i][idx[j]];
    }
    return M;
  }
  /// create identity matrix
  static Basic_Matrix eye(const int32_t m) {
    Basic_Matrix M(m, m);
    for (int32_t i = 0; i < m; ++i)
      M.val[i][i] = T(1);
    return M;
  }

  void eye() {
    zero();
    int32_t min_d = std::min(m, n);
    for (int32_t i = 0; i < min_d; ++i)
      val[i][i] = T(1);
  }

  /// create matrix with ones
  static Basic_Matrix ones(const int32_t m, const int32_t n) {
    Basic_Matrix M(m, n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        M.val[i][j] = T(1);
    return M;
  }
  /// create diagonal matrix with nx1 or 1xn matrix M as elements
  static Basic_Matrix diag(const Basic_Matrix& M) {
    assert((M.m > 1 && M.n == 1) || (M.m == 1 && M.n > 1));
    if (M.n == 1) {
      Basic_Matrix D(M.m, M.m);
      for (int32_t i = 0; i < M.m; ++i)
        D.val[i][i] = M.val[i][0];
      return D;
    } else {
      Basic_Matrix D(M.n, M.n);
      for (int32_t i = 0; i < M.n; ++i)
        D.val[i][i] = M.val[0][i];
      return D;
    }
  }

  /// returns the m-by-n matrix whose elements are taken column-wise from M
  static Basic_Matrix reshape(const Basic_Matrix& M, const int32_t m_, const int32_t n_) {
    assert(M.m * M.n == m_ * n_);
    Basic_Matrix M2(m_, n_);
    int32_t size_ = m_ * n_;
    for (int32_t k = 0; k < size_; ++k) {
      M2.val[k / n_][k % n_] = M.val[k / M.n][k % M.n];
    }
    return M2;
  }

  /// create 3x3 rotation matrices (convention: http://en.wikipedia.org/wiki/Rotation_matrix)
  static Basic_Matrix rotMatX(const tfloat angle) {
    tfloat s = sin(angle);
    tfloat c = cos(angle);
    Basic_Matrix R(3, 3);
    R.val[0][0] = T(+1);
    R.val[1][1] = T(+c);
    R.val[1][2] = T(-s);
    R.val[2][1] = T(+s);
    R.val[2][2] = T(+c);
    return R;
  }
  static Basic_Matrix rotMatY(const tfloat angle) {
    tfloat s = sin(angle);
    tfloat c = cos(angle);
    Basic_Matrix R(3, 3);
    R.val[0][0] = T(+c);
    R.val[0][2] = T(+s);
    R.val[1][1] = T(+1);
    R.val[2][0] = T(-s);
    R.val[2][2] = T(+c);
    return R;
  }
  static Basic_Matrix rotMatZ(const tfloat angle) {
    tfloat s = sin(angle);
    tfloat c = cos(angle);
    Basic_Matrix R(3, 3);
    R.val[0][0] = T(+c);
    R.val[0][1] = T(-s);
    R.val[1][0] = T(+s);
    R.val[1][1] = T(+c);
    R.val[2][2] = T(+1);
    return R;
  }

  Basic_Matrix operator+(const Basic_Matrix& B) const {
    assert(m == B.m && n == B.n);
    Basic_Matrix C(m, n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        C.val[i][j] = val[i][j] + B.val[i][j];
    return C;
  }
  Basic_Matrix& operator+=(const Basic_Matrix& B) {
    assert(m == B.m && n == B.n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        val[i][j] += B.val[i][j];
    return *this;
  }

  Basic_Matrix operator-(const Basic_Matrix& B) const {
    assert(m == B.m && n == B.n);
    Basic_Matrix C(m, n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        C.val[i][j] = val[i][j] - B.val[i][j];
    return C;
  }
  Basic_Matrix& operator-=(const Basic_Matrix& B) {
    assert(m == B.m && n == B.n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        val[i][j] -= B.val[i][j];
    return *this;
  }

  Basic_Matrix operator*(const Basic_Matrix& B) const {
    assert(n == B.m);
    Basic_Matrix C(m, B.n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < B.n; ++j)
        for (int32_t k = 0; k < n; ++k)
          C.val[i][j] += val[i][k] * B.val[k][j];
    return C;
  }
  Basic_Matrix& operator*=(const Basic_Matrix& B) {
    assert(n == B.m);
    Basic_Matrix C(m, B.n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < B.n; ++j)
        for (int32_t k = 0; k < n; ++k)
          C.val[i][j] += val[i][k] * B.val[k][j];

    return this->swap(C);
  }

  Basic_Matrix operator*(const T& s) const {
    Basic_Matrix C(m, n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        C.val[i][j] = val[i][j] * s;
    return C;
  }
  Basic_Matrix& operator*=(const T& s) {
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        val[i][j] *= s;
    return *this;
  }

  /// divide by scalar, make sure s!=0
  Basic_Matrix operator/(const T& s) const {
    Basic_Matrix C(m, n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        C.val[i][j] = val[i][j] / s;
    return C;
  }
  Basic_Matrix& operator/=(const T& s) {
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        val[i][j] /= s;
    return *this;
  }
  /// negative matrix
  Basic_Matrix operator-() const {
    Basic_Matrix C(m, n);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        C.val[i][j] = -val[i][j];
    return C;
  }
  /// transpose
  Basic_Matrix operator~() const {
    Basic_Matrix C(n, m);
    for (int32_t i = 0; i < m; ++i)
      for (int32_t j = 0; j < n; ++j)
        C.val[j][i] = val[i][j];
    return C;
  }
  /// pow
  static Basic_Matrix pow(const Basic_Matrix& value, uint32_t n) {
    Basic_Matrix tmp = value, re(value.m, value.n);
    re.eye();
    for (; n; n >>= 1) {
      if (n & 1)
        re *= tmp;
      tmp *= tmp;
    }
    return re;
  }

  /// print matrix to stream
  friend std::ostream& operator<<(std::ostream& out, const Basic_Matrix& M) {
    if (M.m == 0 || M.n == 0) {
      out << "[empty matrix]";
    } else {
      for (int32_t i = 0; i < M.m; ++i) {
        for (int32_t j = 0; j < M.n; ++j) {
          out << M.val[i][j];
          if (j != M.n - 1) out << "\t";
        }
        out << std::endl;
      }
    }
    return out;
  }

 private:
  void _allocateMemory(const int32_t m_, const int32_t n_) {
    assert((m_ * n_ < MAXSIZE) && val == NULL);
    m = m_;
    n = n_;
    if (m == 0 || n == 0) return;

    val = (T**)malloc(m * sizeof(T*));
    val[0] = (T*)calloc(m * n, sizeof(T));
    for (int32_t i = 1; i < m; ++i)
      val[i] = val[i - 1] + n;
  }

  void _releaseMemory() {
    if (val != NULL) {
      free(val[0]);
      free(val);
    }
  }
};

template <typename T>
void swap(Basic_Matrix<T>& a, Basic_Matrix<T>& b) {
  a.swap(b);
}

typedef Basic_Matrix<tfloat> Matrix;
typedef Basic_Matrix<int32_t> Matrix_i;
typedef Basic_Matrix<uint32_t> Matrix_u;
typedef Basic_Matrix<Complex> Matrix_c;
}  // namespace ytlib
