/**
 * @file shared_buf.hpp
 * @brief 智能buffer
 * @note 基于std::shared_ptr<char[]>的智能buffer
 * @author WT
 * @date 2021-06-04
 */
#pragma once

#include <memory>
#include <string>

// todo: gcc支持c++20 make_shared array语法后统一换成第一种方式
#if defined(_WIN32)
  #define MAKE_SHARED_ARRAY(buf_size) std::make_shared<char[]>(buf_size);
  #define BUF_TYPE char[]
#else
  #define MAKE_SHARED_ARRAY(buf_size) std::shared_ptr<char>(new char[buf_size], [](char* p) { delete[] p; });
  #define BUF_TYPE char
#endif

namespace ytlib {
/**
 * @brief 智能buffer
 * @note 基于std::shared_ptr<char[]>的智能buffer
 */
struct SharedBuf {
  /**
   * @brief 默认构造函数
   */
  SharedBuf() {}

  /**
   * @brief 构造函数
   * @param input_buf_size 输入buf的大小
   */
  SharedBuf(uint32_t input_buf_size) : buf_size(input_buf_size) {
    buf = MAKE_SHARED_ARRAY(buf_size);
  }

  /**
   * @brief 构造函数
   * @note 浅拷贝
   * @param input_buf 输入buf，const std::shared_ptr<char[]>&形式
   * @param input_buf_size 输入buf的大小
   */
  SharedBuf(const std::shared_ptr<BUF_TYPE>& input_buf, uint32_t input_buf_size) : buf(input_buf), buf_size(input_buf_size) {}

  /**
   * @brief 构造函数
   * @note 深拷贝
   * @param input_buf 输入buf，const char*形式
   * @param input_buf_size 输入buf的大小
   */
  SharedBuf(const char* input_buf, uint32_t input_buf_size) : buf_size(input_buf_size) {
    buf = MAKE_SHARED_ARRAY(buf_size);
    memcpy(buf.get(), input_buf, buf_size);
  }

  /**
   * @brief 构造函数
   * @note 深拷贝
   * @param buf_str 输入buf，std::string形式
   */
  SharedBuf(const std::string& buf_str) : buf_size(static_cast<uint32_t>(buf_str.size())) {
    buf = MAKE_SHARED_ARRAY(buf_size);
    memcpy(buf.get(), buf_str.c_str(), buf_size);
  }

  //获取buf大小
  const uint32_t& Size() const { return buf_size; }

  //获取内部shared_ptr<char[]>
  const std::shared_ptr<BUF_TYPE>& GetSharedPtr() const { return buf; }

  //获取char*指针
  char* Get() const { return buf.get(); }

  //默认的拷贝构造函数的浅拷贝，此处提供深拷贝
  static SharedBuf GetDeepCopy(const SharedBuf& sbuf) {
    return SharedBuf(sbuf.buf.get(), sbuf.buf_size);
  }

 private:
  uint32_t buf_size = 0;          ///< buf大小
  std::shared_ptr<BUF_TYPE> buf;  ///< buf智能指针
};

}  // namespace ytlib