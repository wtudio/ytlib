/**
 * @file fileobj_kv.hpp
 * @brief k-v形式文件
 * @details 将key-value型文件与map<string, string>关联
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "fileobj_inf.hpp"

#include <fstream>
#include <map>

namespace ytlib {
/**
 * @brief k-v形式文件类
 * 使用map格式。涉及到存储的都弄成string。以#开头的行为注释
 */
class KeyValueFile : public FileObj<std::map<std::string, std::string> > {
 public:
  KeyValueFile() : FileObj() {}
  virtual ~KeyValueFile() {}

 protected:
  ///从打开的文件中解析获取文件内容结构体
  virtual bool ParseFileObj() {
    std::ifstream infile(filepath_, std::ios::in);
    if (!infile) return false;

    obj_ptr_ = std::make_shared<std::map<std::string, std::string> >();

    std::string buf;
    for (; getline(infile, buf);) {
      std::size_t pos0 = buf.find_first_not_of(' ');

      //以#开头的行为注释
      if (buf[pos0] == '#') continue;

      //key不能为空
      std::size_t pos1 = buf.find_first_of('=', pos0);
      if (pos1 == std::string::npos || pos1 == pos0) continue;
      const std::string& key = buf.substr(pos0, buf.find_last_not_of(' ', pos1 - 1) + 1 - pos0);

      //val可以为空
      std::size_t pos2 = buf.find_first_not_of(' ', pos1 + 1);
      std::size_t pos3 = buf.find_last_not_of(' ');
      const std::string& val = (pos2 < pos3) ? buf.substr(pos2, pos3 + 1 - pos2) : "";

      obj_ptr_->emplace(key, val);
    }

    infile.close();
    return true;
  }

  ///将当前的文件内容结构体保存为文件
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
