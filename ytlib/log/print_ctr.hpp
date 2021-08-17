/**
 * @file print_ctr.hpp
 * @author WT
 * @brief 常规打印辅助工具
 * @note 常规打印辅助工具
 * @date 2021-08-09
 */

#pragma once

#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "ytlib/misc/stl_util.hpp"

namespace ytlib {

/**
 * @brief 常规打印辅助工具
 * @note 使用PrintHelper::Ins()获取全局单例
 * 通过if_print_控制是否打印，默认为true
 * 返回待打印字符串，由程序在调用处打印，以保留原始代码位置信息
 * 注意：本工具打印内容通常较长，应仅用于测试/DEBUG环境
 */
class PrintCtr {
 public:
  static PrintCtr& Ins() {
    static PrintCtr instance;
    return instance;
  }

  void SetPrint(bool if_print) { if_print_ = if_print; }
  bool IfPrint() const { return if_print_; }

  /**
   * @brief 打印结构体。自定义打印方式
   * 
   * @tparam T 结构体类型
   * @param msg 额外信息
   * @param obj 结构体
   * @param f 打印方法
   * @return std::string 结果字符串
   */
  template <typename T>
  std::string PrintStruct(const std::string& msg, const T& obj,
                          std::function<std::string(const T&)> f) const {
    if (!if_print_) return "";

    const size_t MAX_LINE_LEN = 32;

    std::stringstream ss;
    ss << msg << ':';

    std::string obj_str;
    if (f) {
      obj_str = f(obj);
      if (obj_str.empty()) obj_str = "<empty string>";
    } else {
      obj_str = "<unable to print>";
    }

    if (obj_str.length() > MAX_LINE_LEN || obj_str.find('\n') != std::string::npos) {
      ss << '\n';
    }

    ss << obj_str << '\n';

    return ss.str();
  }

  /**
   * @brief 打印结构体。使用结构体的 << 方法
   * 
   * @tparam T 结构体类型
   * @param msg 额外信息
   * @param obj 结构体
   * @return std::string 结果字符串
   */
  template <typename T>
  std::string PrintStruct(const std::string& msg, const T& obj) const {
    std::function<std::string(const T&)> f = [](const T& obj) {
      std::stringstream ss;
      ss << obj;
      return ss.str();
    };
    return PrintStruct(msg, obj, f);
  }

  /**
   * @brief 打印vector。自定义打印方式
   * 
   * @tparam T vector模板参数
   * @param msg 额外信息
   * @param v 待打印vector
   * @param f 打印方法
   * @return std::string 结果字符串
   */
  template <typename T>
  std::string PrintVec(const std::string& msg, const std::vector<T>& v,
                       std::function<std::string(const T&)> f) const {
    return if_print_ ? (msg + ":\n" + Vec2Str(v, f)) : "";
  }

  /**
   * @brief 打印vector。使用T的 << 方法
   * 
   * @tparam T vector模板参数
   * @param msg 额外信息
   * @param v 待打印vector
   * @return std::string 结果字符串
   */
  template <typename T>
  std::string PrintVec(const std::string& msg, const std::vector<T>& v) const {
    return if_print_ ? (msg + ":\n" + Vec2Str(v)) : "";
  }

  /**
   * @brief 打印set。自定义打印方式
   * 
   * @tparam T set模板参数
   * @param msg 额外信息
   * @param s 待打印set
   * @param f 打印方法
   * @return std::string 结果字符串
   */
  template <typename T>
  std::string PrintSet(const std::string& msg, const std::set<T>& s,
                       std::function<std::string(const T&)> f) const {
    return if_print_ ? (msg + ":\n" + Set2Str(s, f)) : "";
  }

  /**
   * @brief 打印set。使用T的 << 方法
   * 
   * @tparam T set模板参数
   * @param msg 额外信息
   * @param s 待打印set
   * @return std::string 结果字符串
   */
  template <typename T>
  std::string PrintSet(const std::string& msg, const std::set<T>& s) const {
    return if_print_ ? (msg + ":\n" + Set2Str(s)) : "";
  }

  /**
   * @brief 打印map。自定义打印方式
   * 
   * @tparam KeyType map中key类型
   * @tparam ValType map中val类型
   * @param msg 额外信息
   * @param m 待打印map
   * @param fkey key打印方法
   * @param fval val打印方法
   * @return std::string 结果字符串
   */
  template <typename KeyType, typename ValType>
  std::string PrintMap(const std::string& msg, const std::map<KeyType, ValType>& m,
                       std::function<std::string(const KeyType&)> fkey,
                       std::function<std::string(const ValType&)> fval) const {
    return if_print_ ? (msg + ":\n" + Map2Str(m, fkey, fval)) : "";
  }

  /**
   * @brief 打印map。使用结构体的 << 方法
   * @param KeyType 模板参数，map中key类型，需支持 << 方法
   * @param ValType 模板参数，map中val类型
   * @param msg 额外信息
   * @param m 待打印map
   * @return 待打印字符串
   */
  template <typename KeyType, typename ValType>
  std::string PrintMap(const std::string& msg, const std::map<KeyType, ValType>& m) const {
    return if_print_ ? (msg + ":\n" + Map2Str(m)) : "";
  }

 private:
  PrintCtr() {}

  bool if_print_ = true;
};

}  // namespace ytlib
