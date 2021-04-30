#pragma once

#include <ctime>
#include <fstream>
#include "log.hpp"

namespace ytlib {

class RotateFileWriter {
 public:
  void Write(const LogData& data);
  int Open(const std::string& file);

 private:
  bool CheckNeedRotate();
  int OpenNewFile();

 private:
  static const std::ofstream::pos_type MAX_FILE_SIZE;

  std::string base_file_name_;  // 基础文件路径
  struct tm t_;                 // 当前文件创建时间
  std::ofstream ofs_;
};

LogWriter GetRotateFileWriter(const std::map<std::string, std::string>& cfg);

}  // namespace ytlib
