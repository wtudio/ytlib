/**
 * @file fileobj_kv.hpp
 * @brief k-v形式文件
 * @note 将key-value型文件与map<string, string>关联
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <fstream>
#include <map>

#include "ytlib/file/fileobj_inf.hpp"

namespace ytlib {

/**
 * @brief k-v形式文件类
 * @note 使用map格式。涉及到存储的都弄成string。以#开头的行为注释
 */
class KeyValueFile : public FileObj<std::map<std::string, std::string> > {
 public:
  /// 构造函数
  KeyValueFile() : FileObj() {}

  /// 析构函数
  virtual ~KeyValueFile() {}

 protected:
  /**
   * @brief 从打开的文件中解析获取文件内容结构体
   *
   * @return true
   * @return false
   */
  virtual bool ParseFileObj() {
    std::ifstream infile(filepath_, std::ios::in);
    if (!infile) return false;

    obj_ptr_ = std::make_shared<std::map<std::string, std::string> >();
    std::map<std::string, std::string>& kv_map = *obj_ptr_;

    std::string buf;
    for (; getline(infile, buf);) {
      if (buf.empty()) continue;
      size_t end_pos = buf.find('#');
      if (end_pos != std::string::npos)
        buf.erase(end_pos);

      size_t key_start_pos = buf.find_first_not_of(' ');

      // key不能为空
      size_t sep_pos = buf.find('=', key_start_pos);
      if (sep_pos == std::string::npos || sep_pos == key_start_pos) continue;

      size_t key_end_pos = buf.find_last_not_of(' ', sep_pos - 1) + 1;
      const std::string& key = buf.substr(key_start_pos, key_end_pos - key_start_pos);

      // val可以为空
      size_t val_start_pos = buf.find_first_not_of(' ', sep_pos + 1);
      size_t val_end_pos = buf.find_last_not_of(' ') + 1;
      const std::string& val = (val_start_pos < val_end_pos) ? buf.substr(val_start_pos, val_end_pos - val_start_pos) : "";

      kv_map[key] = val;
    }

    infile.close();
    return true;
  }

  /**
   * @brief 将当前的文件内容结构体保存为文件
   *
   * @return true
   * @return false
   */
  virtual bool SaveFileObj() {
    std::ofstream ofile(filepath_, std::ios::trunc);
    if (!ofile) return false;

    for (const auto& itr : *obj_ptr_) {
      if (itr.first.empty()) continue;
      ofile << itr.first << " = " << itr.second << '\n';
    }

    ofile.close();
    return true;
  }
};
}  // namespace ytlib
