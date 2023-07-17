/**
 * @file heap.hpp
 * @brief 堆
 * @note 堆和相关的算法，仅学习用。stl中有关于heap的函数：make_heap/pop_heap/push_heap/sort_heap
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <stdexcept>
#include <vector>

namespace ytlib {

/**
 * @brief 堆
 *
 * @tparam T
 */
template <typename T>
class Heap {
 public:
  Heap(bool is_min_heap = true) : is_min_heap_(is_min_heap) {}
  Heap(const std::vector<T>& a, bool is_min_heap = true) : container_(a), is_min_heap_(is_min_heap) {
    Adjust();
  }
  Heap(const T* a, size_t sz, bool is_min_heap = true) : is_min_heap_(is_min_heap) {
    container_.reserve(sz);
    container_.assign(a, a + sz);
    Adjust();
  }
  ~Heap() = default;

  /// 使用数组初始化
  void Assign(const T* a, size_t sz) {
    container_.clear();
    container_.reserve(sz);
    container_.assign(a, a + sz);
    Adjust();
  }

  /// 压入
  void Push(const T& val) {
    container_.emplace_back(val);
    AdjustUp(container_.size() - 1);
  }

  /// 弹出堆顶
  void Pop() {
    if (container_.empty()) [[unlikely]]
      throw std::runtime_error("Heap is empty.");

    using std::swap;
    swap(container_[0], container_[container_.size() - 1]);
    container_.pop_back();
    AdjustDown(0);
  }

  /// 调整为堆
  void Adjust() {
    if (container_.empty()) return;
    for (size_t ii = ((container_.size() - 2) >> 1); ii > 0; --ii) {
      AdjustDown(ii);
    }
    AdjustDown(0);
  }

  void AdjustDown(size_t index, size_t len = 0) {
    using std::swap;
    size_t& parent = index;
    size_t child = LeftChildIndex(parent);
    if (len == 0) len = container_.size();
    while (child < len) {
      // 选取左右子节点中大/小的
      if (((child + 1) < len) && ((container_[child + 1] > container_[child]) ^ is_min_heap_)) {
        ++child;
      }
      // 如果子节点大/小于父节点
      if ((container_[child] > container_[parent]) ^ is_min_heap_) {
        swap(container_[child], container_[parent]);
        parent = child;
        child = LeftChildIndex(parent);
      } else
        break;
    }
  }

  void AdjustUp(size_t index) {
    using std::swap;
    size_t& child = index;
    size_t parent = ParentIndex(child);
    while (child > 0) {
      // 如果子节点大/小于父节点
      if ((container_[child] > container_[parent]) ^ is_min_heap_) {
        swap(container_[child], container_[parent]);
        child = parent;
        parent = ParentIndex(child);
      } else
        break;
    }
  }

  /// 堆排序。最小堆倒序排，最大堆正序排，排完序之后堆类型反转
  void Sort() {
    using std::swap;
    for (size_t ii = container_.size() - 1; ii > 0; --ii) {
      swap(container_[0], container_[ii]);
      AdjustDown(0, ii);
    }
    is_min_heap_ = !is_min_heap_;  // 类型反转
  }

 private:
  size_t LeftChildIndex(size_t index) {
    return (index << 1) + 1;
  }

  size_t RightChildIndex(size_t index) {
    return (index << 1) + 2;
  }

  size_t ParentIndex(size_t index) {
    return (index - 1) >> 1;
  }

 public:
  bool is_min_heap_;
  std::vector<T> container_;  // 可以直接公开访问
};

}  // namespace ytlib
