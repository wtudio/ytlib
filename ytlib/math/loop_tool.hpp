/**
 * @file loop_tool.hpp
 * @author WT
 * @brief 多层循环工具
 * @note 多层循环时的工具，可自定义每层循环数
 * @date 2019-07-26
 */
#pragma once

#include <vector>

namespace ytlib {

/**
 * @brief 循环辅助工具
 * @note 可以实现n层循环。虽然一般不要出现n层循环
 * 使用时：
 *  LoopTool lt;
 *  do{
 *  ...
 *  }while(--lt);
 */
class LoopTool {
 public:
  explicit LoopTool(const std::vector<uint32_t>& input_up_vec) : up_vec(input_up_vec),
                                                                 content_vec(up_vec.size()) {}

  ///++i
  LoopTool& operator++() {
    size_t len = up_vec.size();
    // 从低位开始加
    for (size_t ii = 0; ii < len; ++ii) {
      content_vec[ii] += 1;
      if (content_vec[ii] == up_vec[ii]) {
        content_vec[ii] = 0;
      } else {
        return *this;
      }
    }
    return *this;
  }

  ///--i
  LoopTool& operator--() {
    size_t len = up_vec.size();
    // 从低位开始减
    for (size_t ii = 0; ii < len; ++ii) {
      if (content_vec[ii] == 0) {
        content_vec[ii] = up_vec[ii] - 1;
      } else {
        content_vec[ii] -= 1;
        return *this;
      }
    }
    return *this;
  }

  /// 是否为0。0为false。up_vec为空则始终为0
  operator bool() const {
    size_t len = up_vec.size();
    for (size_t ii = 0; ii < len; ++ii) {
      if (content_vec[ii]) return true;
    }
    return false;
  }

 public:
  std::vector<uint32_t> up_vec;  ///< 进制
  std::vector<uint32_t> content_vec;
};

}  // namespace ytlib
