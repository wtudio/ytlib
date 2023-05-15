/**
 * @file sort_algs.hpp
 * @brief 常用排序算法
 * @note 模板化的排序相关算法，包括冒泡、归并、快排等，所有排序都是从小到大排列。
 * stl中有很多成熟的算法可以直接调用（stl源码剖析p288），此处自己实现的排序算法供学习与改造。
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <algorithm>
#include <cinttypes>
#include <cstring>
#include <map>
#include <set>
#include <vector>

namespace ytlib {

/**
 * @brief 排序工具类
 * @note 重载了比较符号的类，可以继承或改造它来实现自定义类型的排序
 */
class SortObj {
 public:
  SortObj() {}
  explicit SortObj(uint32_t k_) : key(k_) {}
  virtual ~SortObj() {}

  // 关系运算符重载
  bool operator<(const SortObj& val) const { return key < val.key; }
  bool operator>(const SortObj& val) const { return key > val.key; }
  bool operator<=(const SortObj& val) const { return key <= val.key; }
  bool operator>=(const SortObj& val) const { return key >= val.key; }
  bool operator==(const SortObj& val) const { return key == val.key; }
  bool operator!=(const SortObj& val) const { return key != val.key; }

 public:
  uint32_t key = 0;  ///< key
};

/**
 * @brief 冒泡排序
 * @note in-place/稳定
 * @tparam T 待排序数据类型，要保证有大小比较函数
 * @param[inout] arr 待排序数组头指针
 * @param[in] len 待排序数组大小
 */
template <typename T>
void BubbleSort(T* arr, size_t len) {
  if (len < 2) return;
  using std::swap;
  for (size_t ii = 0; ii < len; ++ii) {
    for (size_t jj = 0; jj < len - 1 - ii; ++jj) {
      if (arr[jj] > arr[jj + 1]) {
        swap(arr[jj], arr[jj + 1]);
      }
    }
  }
}

/**
 * @brief 归并排序
 * @note out-place/稳定，递归形式
 * @tparam T 待排序数据类型，要保证有大小比较函数
 * @param[inout] arr 待排序数组头指针
 * @param[in] len 待排序数组大小
 */
template <typename T>
void MergeSort(T* arr, size_t len) {
  if (len < 2) return;
  if (len == 2) {
    using std::swap;
    if (arr[0] > arr[1]) swap(arr[0], arr[1]);
    return;
  }
  size_t middle = len / 2;
  MergeSort(arr, middle);
  MergeSort(arr + middle, len - middle);

  std::vector<T> tmp_vec(len);
  size_t lc = 0, rc = middle, tc = 0;
  while ((lc < middle) && (rc < len)) {
    tmp_vec[tc++] = (arr[lc] < arr[rc]) ? arr[lc++] : arr[rc++];
  }
  if (rc == len) memcpy(arr + tc, arr + lc, (middle - lc) * sizeof(T));
  memcpy(arr, tmp_vec.data(), tc * sizeof(T));
}

/**
 * @brief 归并排序
 * @note out-place/稳定，非递归形式，空间复杂度O(n)
 * @tparam T 待排序数据类型，要保证有大小比较函数
 * @param[inout] arr 待排序数组头指针
 * @param[in] len 待排序数组大小
 */
template <typename T>
void MergeSort2(T* arr, size_t len) {
  if (len < 2) return;
  using std::swap;
  if (len == 2) {
    if (arr[0] > arr[1]) swap(arr[0], arr[1]);
    return;
  }

  std::vector<T> tmp_vec(len);
  T *tmp_arr1 = tmp_vec.data(), *tmp_arr0 = arr;
  for (size_t ii = 1; ii < len; ii <<= 1) {
    size_t jj = 0;
    while (jj + ii < len) {
      size_t lc = jj, rc = jj + ii, tc = jj, middle = jj + ii, last = jj + 2 * ii;
      if (last > len) last = len;

      while ((lc < middle) && (rc < last)) {
        tmp_arr1[tc++] = (tmp_arr0[lc] < tmp_arr0[rc]) ? tmp_arr0[lc++] : tmp_arr0[rc++];
      }
      if (rc == last)
        memcpy(tmp_arr1 + tc, tmp_arr0 + lc, (middle - lc) * sizeof(T));
      else if (lc == middle)
        memcpy(tmp_arr1 + tc, tmp_arr0 + rc, (last - rc) * sizeof(T));

      if (last == len) {
        jj = len;
        break;
      }
      jj += 2 * ii;
    }
    if (jj < len) {
      memcpy(tmp_arr1 + jj, tmp_arr0 + jj, (len - jj) * sizeof(T));
    }
    swap(tmp_arr1, tmp_arr0);
  }
  if (tmp_arr0 != arr) {
    memcpy(arr, tmp_arr0, len * sizeof(T));
  }
}

/**
 * @brief 快速排序
 * @note in-place/不稳定
 * @tparam T 待排序数据类型，要保证有大小比较函数
 * @param[inout] arr 待排序数组头指针
 * @param[in] len 待排序数组大小
 */
template <typename T>
void QuickSort(T* arr, size_t len) {
  if (len < 2) return;
  using std::swap;
  if (len == 2) {
    if (arr[0] > arr[1]) swap(arr[0], arr[1]);
    return;
  }
  size_t first = 0, last = len - 1, cur = first;
  while (first < last) {
    while (first < last && arr[last] >= arr[cur]) --last;
    if (cur != last) {
      swap(arr[cur], arr[last]);
      cur = last;
    }
    while (first < last && arr[first] <= arr[cur]) ++first;
    if (cur != first) {
      swap(arr[cur], arr[first]);
      cur = first;
    }
  }
  QuickSort(arr, cur);
  QuickSort(arr + cur + 1, len - cur - 1);
}

/**
 * @brief 二分查找
 * @note 应用于从小到大排序好的数组中，如有重复则找首先出现的那个
 * @tparam T 待查找数据类型，要保证有大小比较函数
 * @param[in] arr 待查找数组头指针，需保证已经从小到大排序
 * @param[in] len 待查找数组大小
 * @param[in] key 待查找数据
 * @return size_t 数据key在数组arr中首次出现的位置，没找到则返回len
 */
template <typename T>
size_t BinarySearch(T* arr, size_t len, const T& key) {
  if (len == 0) return 0;
  size_t low = 0, high = len - 1;
  while (low < high) {
    size_t mid = low + (high - low) / 2;
    if (arr[mid] < key)
      low = mid + 1;
    else
      high = mid;
  }
  if (arr[low] == key) return low;
  return len;  // 没找到，返回len
}

/**
 * @brief 二分查找
 * @note 应用于从小到大排序好的数组中，如有重复则找最后出现的那个
 * @tparam T 待查找数据类型，要保证有大小比较函数
 * @param[in] arr 待查找数组头指针，需保证已经从小到大排序
 * @param[in] len 待查找数组大小
 * @param[in] key 待查找数据
 * @return size_t 数据key在数组arr中首次出现的位置，没找到则返回len
 */
template <typename T>
size_t BinarySearchLast(T* arr, size_t len, const T& key) {
  if (len == 0) return 0;
  size_t low = 0, high = len - 1;
  while (low < high) {
    size_t mid = low + (high - low + 1) / 2;
    if (arr[mid] <= key)
      low = mid;
    else
      high = mid - 1;
  }
  if (arr[high] == key) return high;
  return len;  // 没找到，返回len
}

}  // namespace ytlib
