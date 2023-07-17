/**
 * @file matrix.hpp
 * @author WT
 * @brief 矩阵类
 * @note 基于模板的简易矩阵类型
 * @date 2019-07-26
 */
#pragma once

#include <cmath>
#include <cstring>
#include <iostream>
#include <string>

namespace ytlib {
/**
 * @brief 简易矩阵类
 * @note 模板形式的简易矩阵类，可以放复数、int、float、double、自定义类型等等
 */
template <typename T = double>
class Basic_Matrix {
 public:
  Basic_Matrix() = default;
  Basic_Matrix(const uint32_t input_max_row, const uint32_t input_max_col) {
    AllocateMemory(input_max_row, input_max_col, true);
  }
  Basic_Matrix(const Basic_Matrix& M) {
    AllocateMemory(M.max_row, M.max_col);
    if (val != nullptr) memcpy(val[0], M.val[0], max_col * max_row * sizeof(T));
  }
  Basic_Matrix(Basic_Matrix&& M) : val(M.val), max_row(M.max_row), max_col(M.max_col) {
    M.val = nullptr;
    M.max_row = M.max_col = 0;
  }

  Basic_Matrix(const uint32_t input_max_row, const uint32_t input_max_col,
               const std::vector<T>& input_data) {
    AllocateMemory(input_max_row, input_max_col);
    Assgin(input_data);
  }
  Basic_Matrix(const uint32_t input_max_row, const uint32_t input_max_col,
               const std::vector<std::vector<T> >& input_data) {
    AllocateMemory(input_max_row, input_max_col);
    Assgin(input_data);
  }

  ~Basic_Matrix() { ReleaseMemory(); }

  Basic_Matrix& operator=(const Basic_Matrix& M) {
    if (this != &M) {
      if (M.max_row != max_row || M.max_col != max_col) {
        ReleaseMemory();
        AllocateMemory(M.max_row, M.max_col);
      }
      if (val != nullptr) memcpy(val[0], M.val[0], max_col * max_row * sizeof(T));
    }
    return *this;
  }

  Basic_Matrix& operator=(Basic_Matrix&& M) {
    if (this != &M) {
      // 先释放自己的资源
      ReleaseMemory();
      val = M.val;
      max_row = M.max_row;
      max_col = M.max_col;
      M.val = nullptr;
      M.max_row = M.max_col = 0;
    }
    return *this;
  }

  void Assgin(const std::vector<T>& input_data) {
    if (val == nullptr) return;
    memcpy(val[0], input_data.data(),
           std::min(max_row * max_col, static_cast<uint32_t>(input_data.size())) * sizeof(T));
  }

  void Assgin(const std::vector<std::vector<T> >& input_data) {
    uint32_t real_row = std::min(max_row, static_cast<uint32_t>(input_data.size()));
    for (uint32_t cur_row = 0; cur_row < real_row; ++cur_row) {
      uint32_t real_col = std::min(max_col, static_cast<uint32_t>(input_data[cur_row].size()));
      for (uint32_t cur_col = 0; cur_col < real_col; ++cur_col) {
        val[cur_row][cur_col] = input_data[cur_row][cur_col];
      }
    }
  }

  /**
   * @brief 将矩阵中部分区域数据按行展开
   *
   * @param[in] row_begin
   * @param[in] col_begin
   * @param[in] row_end
   * @param[in] col_end
   * @return std::vector<T> 展开后的数据
   */
  std::vector<T> GetData(uint32_t row_begin = 0, uint32_t col_begin = 0,
                         uint32_t row_end = UINT32_MAX, uint32_t col_end = UINT32_MAX) const {
    std::vector<T> ret;

    uint32_t real_row_end = (row_end >= max_row) ? max_row : (row_end + 1);
    uint32_t real_col_end = (col_end >= max_col) ? max_col : (col_end + 1);

    if (row_begin >= real_row_end || col_begin >= real_col_end) return ret;

    ret.resize((real_row_end - row_begin) * (real_col_end - col_begin));
    uint32_t ct = 0;
    for (uint32_t cur_row = row_begin; cur_row < real_row_end; ++cur_row) {
      for (uint32_t cur_col = col_begin; cur_col < real_col_end; ++cur_col) {
        ret[ct++] = val[cur_row][cur_col];
      }
    }
    return ret;
  }

  /**
   * @brief 获取子矩阵
   *
   * @param[in] row_begin
   * @param[in] col_begin
   * @param[in] row_end
   * @param[in] col_end
   * @return Basic_Matrix 子矩阵
   */
  Basic_Matrix GetMat(uint32_t row_begin, uint32_t col_begin,
                      uint32_t row_end = UINT32_MAX, uint32_t col_end = UINT32_MAX) const {
    Basic_Matrix M;

    uint32_t real_row_end = (row_end >= max_row) ? max_row : (row_end + 1);
    uint32_t real_col_end = (col_end >= max_col) ? max_col : (col_end + 1);
    if (row_begin >= real_row_end || col_begin >= real_col_end) return M;

    M.AllocateMemory(real_row_end - row_begin, real_col_end - col_begin);
    for (uint32_t cur_row = 0; cur_row < M.max_row; ++cur_row) {
      for (uint32_t cur_col = 0; cur_col < M.max_col; ++cur_col) {
        M.val[cur_row][cur_col] = val[row_begin + cur_row][col_begin + cur_col];
      }
    }

    return M;
  }

  /**
   * @brief 设置子矩阵
   *
   * @param[in] M
   * @param[in] row_begin
   * @param[in] col_begin
   * @param[in] row_end
   * @param[in] col_end
   */
  void SetMat(const Basic_Matrix& M, const uint32_t row_begin = 0, const uint32_t col_begin = 0,
              uint32_t row_end = UINT32_MAX, uint32_t col_end = UINT32_MAX) {
    uint32_t real_row_end = (row_end >= max_row) ? max_row : (row_end + 1);
    uint32_t real_col_end = (col_end >= max_col) ? max_col : (col_end + 1);

    uint32_t tmp_max_row = std::min(real_row_end, row_begin + M.max_row);
    uint32_t tmp_max_col = std::min(real_col_end, col_begin + M.max_col);
    for (uint32_t cur_row = row_begin; cur_row < tmp_max_row; ++cur_row) {
      for (uint32_t cur_col = col_begin; cur_col < tmp_max_col; ++cur_col) {
        val[cur_row][cur_col] = M.val[cur_row - row_begin][cur_col - col_begin];
      }
    }
  }

  /**
   * @brief 将矩阵一定区域内都设置为一个常量
   *
   * @param[in] in_val
   * @param[in] row_begin
   * @param[in] col_begin
   * @param[in] row_end
   * @param[in] col_end
   */
  void SetVal(const T& in_val, uint32_t row_begin = 0, uint32_t col_begin = 0,
              uint32_t row_end = UINT32_MAX, uint32_t col_end = UINT32_MAX) {
    uint32_t real_row_end = (row_end >= max_row) ? max_row : (row_end + 1);
    uint32_t real_col_end = (col_end >= max_col) ? max_col : (col_end + 1);

    if (row_begin >= real_row_end || col_begin >= real_col_end) return;

    for (uint32_t cur_row = row_begin; cur_row < real_row_end; ++cur_row) {
      for (uint32_t cur_col = col_begin; cur_col < real_col_end; ++cur_col) {
        val[cur_row][cur_col] = in_val;
      }
    }
  }

  /**
   * @brief 设置对角矩阵
   *
   * @param[in] in_val
   * @param[in] idx_begin
   * @param[in] idx_end
   */
  void SetDiag(const T& in_val, uint32_t idx_begin = 0, uint32_t idx_end = UINT32_MAX) {
    uint32_t real_idx_end = (idx_end >= max_row || idx_end >= max_col) ? std::min(max_row, max_col) : (idx_end + 1);

    if (idx_begin >= real_idx_end) return;

    for (uint32_t ii = idx_begin; ii < real_idx_end; ++ii)
      val[ii][ii] = in_val;
  }

  /**
   * @brief 设置对角矩阵
   *
   * @param[in] input_vec
   * @param[in] idx_begin
   * @param[in] idx_end
   */
  void SetDiag(const std::vector<T>& input_vec, uint32_t idx_begin = 0, uint32_t idx_end = UINT32_MAX) {
    uint32_t real_idx_end = (idx_end >= max_row || idx_end >= max_col) ? std::min(max_row, max_col) : (idx_end + 1);

    if (idx_begin >= real_idx_end) return;

    uint32_t ct = 0;
    for (uint32_t ii = idx_begin; ii < real_idx_end; ++ii) {
      if (ct >= input_vec.size()) return;
      val[ii][ii] = input_vec[ct++];
    }
  }

  /**
   * @brief 设置零矩阵
   *
   */
  void Zero() {
    if (val == nullptr) return;
    memset(val[0], 0, max_row * max_col * sizeof(T));
  }

  Basic_Matrix& Swap(Basic_Matrix& M) {
    if (this != &M) {
      uint32_t tmp = M.max_row;
      M.max_row = this->max_row;
      this->max_row = tmp;
      tmp = M.max_col;
      M.max_col = this->max_col;
      this->max_col = tmp;
      T** ptmp = M.val;
      M.val = this->val;
      this->val = ptmp;
    }
    return *this;
  }

  bool operator==(const Basic_Matrix& M) const {
    if (max_row != M.max_row) return false;
    if (max_col != M.max_col) return false;
    if (max_row > 0 && max_col > 0) {
      uint32_t len = max_row * max_col;
      for (uint32_t ii = 0; ii < len; ++ii) {
        if (val[0][ii] != M.val[0][ii]) return false;
      }
    }

    return true;
  }
  bool operator!=(const Basic_Matrix& M) const {
    return !(*this == M);
  }

  Basic_Matrix operator+(const Basic_Matrix& M) const {
    if (max_row != M.max_row || max_col != M.max_col)
      throw std::logic_error("The matrix must be the same size.");

    Basic_Matrix ret(max_row, max_col);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        ret.val[cur_row][cur_col] = val[cur_row][cur_col] + M.val[cur_row][cur_col];
    return ret;
  }
  Basic_Matrix& operator+=(const Basic_Matrix& M) {
    if (max_row != M.max_row || max_col != M.max_col)
      throw std::logic_error("The matrix must be the same size.");

    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        val[cur_row][cur_col] += M.val[cur_row][cur_col];
    return *this;
  }

  Basic_Matrix operator-(const Basic_Matrix& M) const {
    if (max_row != M.max_row || max_col != M.max_col)
      throw std::logic_error("The matrix must be the same size.");

    Basic_Matrix ret(max_row, max_col);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        ret.val[cur_row][cur_col] = val[cur_row][cur_col] - M.val[cur_row][cur_col];
    return ret;
  }
  Basic_Matrix& operator-=(const Basic_Matrix& M) {
    if (max_row != M.max_row || max_col != M.max_col)
      throw std::logic_error("The matrix must be the same size.");

    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        val[cur_row][cur_col] -= M.val[cur_row][cur_col];
    return *this;
  }

  Basic_Matrix operator*(const Basic_Matrix& M) const {
    if (max_col != M.max_row)
      throw std::logic_error("M.max_row must be equal to this->max_col.");

    Basic_Matrix ret(max_row, M.max_col);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < M.max_col; ++cur_col)
        for (uint32_t k = 0; k < max_col; ++k)
          ret.val[cur_row][cur_col] += val[cur_row][k] * M.val[k][cur_col];
    return ret;
  }
  Basic_Matrix& operator*=(const Basic_Matrix& M) {
    if (max_col != M.max_row)
      throw std::logic_error("M.max_row must be equal to this->max_col.");

    Basic_Matrix ret(max_row, M.max_col);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < M.max_col; ++cur_col)
        for (uint32_t k = 0; k < max_col; ++k)
          ret.val[cur_row][cur_col] += val[cur_row][k] * M.val[k][cur_col];

    return this->Swap(ret);
  }

  Basic_Matrix operator*(const T& in_val) const {
    Basic_Matrix ret(max_row, max_col);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        ret.val[cur_row][cur_col] = val[cur_row][cur_col] * in_val;
    return ret;
  }
  Basic_Matrix& operator*=(const T& in_val) {
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        val[cur_row][cur_col] *= in_val;
    return *this;
  }

  Basic_Matrix operator/(const T& in_val) const {
    Basic_Matrix ret(max_row, max_col);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        ret.val[cur_row][cur_col] = val[cur_row][cur_col] / in_val;
    return ret;
  }
  Basic_Matrix& operator/=(const T& in_val) {
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        val[cur_row][cur_col] /= in_val;
    return *this;
  }

  Basic_Matrix operator-() const {
    Basic_Matrix ret(max_row, max_col);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        ret.val[cur_row][cur_col] = -val[cur_row][cur_col];
    return ret;
  }

  /// 转置
  Basic_Matrix operator~() const {
    Basic_Matrix ret(max_col, max_row);
    for (uint32_t cur_row = 0; cur_row < max_row; ++cur_row)
      for (uint32_t cur_col = 0; cur_col < max_col; ++cur_col)
        ret.val[cur_col][cur_row] = val[cur_row][cur_col];
    return ret;
  }

  /**
   * @brief 创建单位矩阵
   *
   * @param[in] in_val 单位值
   * @param[in] max_idx 矩阵大小
   * @return Basic_Matrix
   */
  static Basic_Matrix Eye(const T& in_val, const uint32_t max_idx) {
    Basic_Matrix M(max_idx, max_idx);
    M.SetDiag(in_val);
    return M;
  }

  /**
   * @brief 乘方
   * @note 矩阵必须长宽相等
   * @param[in] in_val 单位值
   * @param[in] M 底数
   * @param[in] n 次方数
   * @return Basic_Matrix
   */
  static Basic_Matrix Pow(const T& in_val, const Basic_Matrix& M, uint32_t n) {
    Basic_Matrix tmp = M, re(M.max_row, M.max_col);
    re.SetDiag(in_val);
    for (; n; n >>= 1) {
      if (n & 1)
        re *= tmp;
      tmp *= tmp;
    }
    return re;
  }

  friend std::ostream& operator<<(std::ostream& out, const Basic_Matrix& M) {
    if (M.max_row == 0 || M.max_col == 0) {
      out << "[empty matrix]";
    } else {
      out << "[row " << M.max_row << ", col " << M.max_col << "]\n";
      for (uint32_t cur_row = 0; cur_row < M.max_row; ++cur_row) {
        for (uint32_t cur_col = 0; cur_col < M.max_col; ++cur_col) {
          out << M.val[cur_row][cur_col];
          if (cur_col != M.max_col - 1) out << '\t';
        }
        out << '\n';
      }
    }
    return out;
  }

 private:
  void AllocateMemory(const uint32_t input_max_row, const uint32_t input_max_col, bool need_init = false) {
    if (input_max_row == 0 || input_max_col == 0) return;

    max_row = input_max_row;
    max_col = input_max_col;
    val = (T**)malloc(max_row * sizeof(T*));

    if (need_init) {
      val[0] = (T*)calloc(max_row * max_col, sizeof(T));
    } else {
      val[0] = (T*)malloc(max_row * max_col * sizeof(T));
    }

    for (uint32_t ii = 1; ii < max_row; ++ii)
      val[ii] = val[ii - 1] + max_col;
  }

  void ReleaseMemory() {
    if (val != nullptr) {
      free(val[0]);
      free(val);
      val = nullptr;
    }
  }

 public:
  T** val = nullptr;
  uint32_t max_row = 0;  ///< 行。每行的数据是连续的，应尽量使行号为索引
  uint32_t max_col = 0;  ///< 列
};

template <typename T = double>
void swap(Basic_Matrix<T>& a, Basic_Matrix<T>& b) { a.Swap(b); }

using Matrix = Basic_Matrix<double>;
using Matrix_f = Basic_Matrix<float>;
using Matrix_i32 = Basic_Matrix<int32_t>;
using Matrix_u32 = Basic_Matrix<uint32_t>;
using Matrix_i64 = Basic_Matrix<int64_t>;
using Matrix_u64 = Basic_Matrix<uint64_t>;

/**
 * @brief 获取3x3旋转矩阵
 * @note 参考http://en.wikipedia.org/wiki/Rotation_matrix
 * @param[in] angle
 * @return Matrix
 */
inline Matrix RotMatX(const double angle) {
  double s = std::sin(angle);
  double c = std::cos(angle);
  Matrix R(3, 3);
  R.val[0][0] = 1.0;
  R.val[1][1] = c;
  R.val[1][2] = -s;
  R.val[2][1] = s;
  R.val[2][2] = c;
  return R;
}

/**
 * @brief 获取3x3旋转矩阵
 * @note 参考http://en.wikipedia.org/wiki/Rotation_matrix
 * @param[in] angle
 * @return Matrix
 */
inline Matrix RotMatY(const double angle) {
  double s = std::sin(angle);
  double c = std::cos(angle);
  Matrix R(3, 3);
  R.val[0][0] = c;
  R.val[0][2] = s;
  R.val[1][1] = 1.0;
  R.val[2][0] = -s;
  R.val[2][2] = c;
  return R;
}

/**
 * @brief 获取3x3旋转矩阵
 * @note 参考http://en.wikipedia.org/wiki/Rotation_matrix
 * @param[in] angle
 * @return Matrix
 */
inline Matrix RotMatZ(const double angle) {
  double s = std::sin(angle);
  double c = std::cos(angle);
  Matrix R(3, 3);
  R.val[0][0] = c;
  R.val[0][1] = -s;
  R.val[1][0] = s;
  R.val[1][1] = c;
  R.val[2][2] = 1.0;
  return R;
}

}  // namespace ytlib
