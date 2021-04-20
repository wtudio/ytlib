/**
 * @file Heap.h
 * @brief 堆
 * @details 堆和相关的算法。stl中有关于heap的函数：make_heap/pop_heap/push_heap/sort_heap
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <cassert>
#include <vector>

namespace ytlib {

#define LEFT_CHILD(x) ((x << 1) + 1)
#define RIGHT_CHILD(x) ((x << 1) + 2)
#define PARENT(x) ((x - 1) >> 1)

/**
 * @brief 堆
 * type=true为最小堆，否则最大堆。T需要支持比较运算
 */
template <typename T>
class Heap {
 public:
  Heap(bool _type = true) : type(_type) {}
  ~Heap() {}

  Heap(const std::vector<T>& a, bool _type = true) : container(a), type(_type) {
    adjust();
  }
  Heap(const T* a, size_t sz, bool _type = true) : type(_type) {
    container.reserve(sz);
    container.assign(a, a + sz);
    adjust();
  }
  ///使用数组初始化
  void assign(const T* a, size_t sz) {
    container.clear();
    container.reserve(sz);
    container.assign(a, a + sz);
    adjust();
  }

  ///压入
  void push(const T& val) {
    container.push_back(val);
    adjustUp(container.size() - 1);
  }

  ///弹出堆顶
  void pop() {
    using std::swap;
    assert(!container.empty());
    swap(container[0], container[container.size() - 1]);
    container.pop_back();
    adjustDown(0);
  }

  ///调整为堆
  void adjust() {
    if (container.empty()) return;
    for (size_t ii = ((container.size() - 2) >> 1); ii > 0; --ii) {
      adjustDown(ii);
    }
    adjustDown(0);
  }

  void adjustDown(size_t index, size_t len = 0) {
    using std::swap;
    size_t& parent = index;
    size_t child = LEFT_CHILD(parent);
    if (len == 0) len = container.size();
    while (child < len) {
      //选取左右子节点中大/小的
      if (((child + 1) < len) && ((container[child + 1] > container[child]) ^ type)) {
        ++child;
      }
      //如果子节点大/小于父节点
      if ((container[child] > container[parent]) ^ type) {
        swap(container[child], container[parent]);
        parent = child;
        child = LEFT_CHILD(parent);
      } else
        break;
    }
  }

  void adjustUp(size_t index) {
    using std::swap;
    size_t& child = index;
    size_t parent = PARENT(child);
    while (child > 0) {
      //如果子节点大/小于父节点
      if ((container[child] > container[parent]) ^ type) {
        swap(container[child], container[parent]);
        child = parent;
        parent = PARENT(child);
      } else
        break;
    }
  }
  ///堆排序。最小堆倒序排，最大堆正序排，排完序之后堆类型反转
  void sort() {
    using std::swap;
    for (size_t ii = container.size() - 1; ii > 0; --ii) {
      swap(container[0], container[ii]);
      adjustDown(0, ii);
    }
    type = !type;  //类型反转
  }

  bool type;
  std::vector<T> container;  //可以直接公开访问
};

}  // namespace ytlib
