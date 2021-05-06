/**
 * @file LoopTool.h
 * @brief 多层循环工具
 * @details 多层循环时的工具，但尽量不要用到
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <cassert>
#include <cstdint>
#include <vector>

namespace ytlib {

/**
 * @brief 循环辅助工具
 * 可以实现n层循环。虽然一般不要出现n层循环
 * 使用时：
 *  LoopTool lt;
 *  do{
 *  ...
 *  }while(--lt);
 */
class LoopTool {
 public:
  explicit LoopTool(const std::vector<uint32_t>& up_) : m_vecUp(up_) {
    std::size_t len = m_vecUp.size();
    assert(len);
    m_vecContent.resize(len);
    for (std::size_t ii = 0; ii < len; ++ii) {
      m_vecContent[ii] = 0;
    }
  }

  ///++i
  LoopTool& operator++() {
    std::size_t len = m_vecUp.size();
    //从低位开始加
    for (std::size_t ii = 0; ii < len; ++ii) {
      m_vecContent[ii] += 1;
      if (m_vecContent[ii] == m_vecUp[ii])
        m_vecContent[ii] = 0;
      else
        return *this;
    }
    return *this;
  }
  ///--i
  LoopTool& operator--() {
    std::size_t len = m_vecUp.size();
    //从低位开始减
    for (std::size_t ii = 0; ii < len; ++ii) {
      if (m_vecContent[ii] == 0)
        m_vecContent[ii] = m_vecUp[ii] - 1;
      else {
        m_vecContent[ii] -= 1;
        return *this;
      }
    }
    return *this;
  }
  ///是否为0。0为false
  operator bool() const {
    std::size_t len = m_vecUp.size();
    for (std::size_t ii = 0; ii < len; ++ii) {
      if (m_vecContent[ii]) return true;
    }
    return false;
  }

  std::vector<uint32_t> m_vecContent;
  std::vector<uint32_t> m_vecUp;  ///<进制
};

}  // namespace ytlib
