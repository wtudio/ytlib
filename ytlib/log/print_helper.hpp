#pragma once

#include <functional>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace ytlib {
/**
 * @brief 常规打印辅助工具
 * 使用PrintHelper::Ins()获取全局单例
 * 通过if_print_控制是否打印，默认为true
 * 返回待打印字符串，由程序在调用处打印，以保留原始代码位置信息
 * 注意：本工具打印内容通常较长，应仅用于测试/DEBUG环境
 */
class PrintHelper {
 public:
  static PrintHelper& Ins() {
    static PrintHelper instance;
    return instance;
  }

  void SetPrint(bool if_print) { if_print_ = if_print; }
  bool IfPrint() const { return if_print_; }

  /**
   * @brief 打印结构体。自定义打印方式
   * @param T 模板参数，结构体类型
   * @param msg 额外信息
   * @param obj 结构体
   * @param f 打印方法
   * @return 待打印字符串
   */
  template <typename T>
  std::string PrintStruct(const std::string& msg, const T& obj,
                          std::function<std::string(const T&)> f) const {
    if (!if_print_ || !f) return "";

    std::stringstream ss;
    ss << msg << ':';
    const std::string& obj_str = f(obj);
    if (obj_str.empty())
      ss << "<empty string>";
    else if (obj_str.find('\n') != std::string::npos)
      ss << '\n'
         << obj_str;
    else
      ss << obj_str;

    return ss.str();
  }

  /**
   * @brief 打印结构体。使用结构体的 << 方法
   * @param T 模板参数，结构体类型
   * @param msg 额外信息
   * @param obj 结构体
   * @return 待打印字符串
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
   * @param T 模板参数，vector模板参数
   * @param msg 额外信息
   * @param v 待打印vector
   * @param f 打印方法
   * @return 待打印字符串
   */
  template <typename T>
  std::string PrintVec(const std::string& msg, const std::vector<T>& v,
                       std::function<std::string(const T&)> f) const {
    if (!if_print_ || !f) return "";
    std::stringstream ss;
    ss << msg << ":\nvec size = " << v.size() << '\n';
    size_t ct = 0;
    for (auto& itr : v) {
      ss << "[index=" << ct << "]:";
      const std::string& obj_str = f(itr);
      if (obj_str.empty())
        ss << "<empty string>";
      else if (obj_str.find('\n') != std::string::npos)
        ss << '\n'
           << obj_str;
      else
        ss << obj_str;

      ss << '\n';
      ++ct;
    }
    return ss.str();
  }

  /**
   * @brief 打印vector。使用结构体的 << 方法
   * @param T 模板参数，vector模板参数
   * @param msg 额外信息
   * @param v 待打印vector
   * @return 待打印字符串
   */
  template <typename T>
  std::string PrintVec(const std::string& msg, const std::vector<T>& v) const {
    std::function<std::string(const T&)> f = [](const T& obj) {
      std::stringstream ss;
      ss << obj;
      return ss.str();
    };
    return PrintVec(msg, v, f);
  }

  /**
   * @brief 打印set。自定义打印方式
   * @param T 模板参数，set模板参数
   * @param msg 额外信息
   * @param s 待打印set
   * @param f 打印方法
   * @return 待打印字符串
   */
  template <typename T>
  std::string PrintSet(const std::string& msg, const std::set<T>& s,
                       std::function<std::string(const T&)> f) const {
    if (!if_print_ || !f) return "";
    std::stringstream ss;
    ss << msg << ":\nset size = " << s.size() << '\n';
    size_t ct = 0;
    for (auto& itr : s) {
      ss << "[index=" << ct << "]:";
      const std::string& obj_str = f(itr);
      if (obj_str.empty())
        ss << "<empty string>";
      else if (obj_str.find('\n') != std::string::npos)
        ss << '\n'
           << obj_str;
      else
        ss << obj_str;

      ss << '\n';
      ++ct;
    }
    return ss.str();
  }

  /**
   * @brief 打印set。使用结构体的 << 方法
   * @param T 模板参数，set模板参数
   * @param msg 额外信息
   * @param s 待打印set
   * @return 待打印字符串
   */
  template <typename T>
  std::string PrintSet(const std::string& msg, const std::set<T>& s) const {
    std::function<std::string(const T&)> f = [](const T& obj) {
      std::stringstream ss;
      ss << obj;
      return ss.str();
    };
    return PrintSet(msg, s, f);
  }

  /**
   * @brief 打印map。自定义val打印方式
   * @param KeyType 模板参数，map中key类型，需支持 << 方法
   * @param ValType 模板参数，map中val类型
   * @param msg 额外信息
   * @param m 待打印map
   * @param f 打印方法
   * @return 待打印字符串
   */
  template <typename KeyType, typename ValType>
  std::string PrintMap(const std::string& msg, const std::map<KeyType, ValType>& m,
                       std::function<std::string(const ValType&)> f) const {
    if (!if_print_ || !f) return "";
    std::stringstream ss;
    ss << msg << ":\nmap size = " << m.size() << '\n';
    size_t ct = 0;
    for (auto& itr : m) {
      ss << "[index=" << ct << ", key=" << itr.first << "]:";
      const std::string& obj_str = f(itr.second);
      if (obj_str.empty())
        ss << "<empty string>";
      else if (obj_str.find('\n') != std::string::npos)
        ss << '\n'
           << obj_str;
      else
        ss << obj_str;

      ss << '\n';
      ++ct;
    }
    return ss.str();
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
    std::function<std::string(const ValType&)> f = [](const ValType& obj) {
      std::stringstream ss;
      ss << obj;
      return ss.str();
    };
    return PrintMap(msg, m, f);
  }

 private:
  PrintHelper() {}

  bool if_print_ = true;
};

}  // namespace ytlib
