#pragma once

#include <ctime>
#include <fstream>
#include "log.hpp"
#include "log_exports.h"

namespace ytlib {

class RotateFileWriter {
 public:
  ~RotateFileWriter();

  void Write(const LogData& data);
  int Open(const std::string& file);

  void SetMaxFileSize(size_t max_file_size);

 private:
  bool CheckNeedRotate();
  int OpenNewFile();

 private:
  std::string base_file_name_;  // 基础文件路径
  struct tm t_;                 // 当前文件创建时间
  std::ofstream ofs_;

  std::ofstream::pos_type max_file_size_ = 16 * 1024 * 1024;  // 最大日志文件大小，默认16M
};

LOG_API LogWriter GetRotateFileWriter(const std::map<std::string, std::string>& cfg);

}  // namespace ytlib
