#pragma once

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

 private:
  PrintHelper() {}

  bool if_print_ = true;
};
}  // namespace ytlib
