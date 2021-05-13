#include "rotate_file_writer.hpp"
#include <cstring>
#include <filesystem>

#if defined(_WIN32)
  #define localtime_r(a, b) localtime_s(b, a)
#endif

namespace ytlib {

RotateFileWriter::~RotateFileWriter() {
  if (ofs_.is_open()) {
    ofs_.flush();
    ofs_.clear();
    ofs_.close();
  }
}

void RotateFileWriter::Write(const LogData& data) {
  if (CheckNeedRotate() && (0 != OpenNewFile()))
    return;

  ofs_ << "[" << data.time << "]["
       << static_cast<uint8_t>(data.lvl) << "]["
       << data.thread_id << "]";

  if (data.ctx) {
    for (auto& itr : *(data.ctx))
      ofs_ << "[" << itr.first << "=" << itr.second << "]";
  }

  ofs_ << data.msg << std::endl;
}

int RotateFileWriter::Open(const std::string& file) {
  base_file_name_ = file;
  return OpenNewFile();
}

void RotateFileWriter::SetMaxFileSize(size_t max_file_size) {
  // 至少1M
  if (max_file_size > 1024 * 1024)
    max_file_size_ = max_file_size;
}

bool RotateFileWriter::CheckNeedRotate() {
  auto now = std::chrono::system_clock::now();
  time_t cnow = std::chrono::system_clock::to_time_t(now);
  struct tm now_tm;
  localtime_r(&cnow, &now_tm);
  if (now_tm.tm_hour != t_.tm_hour || now_tm.tm_yday != t_.tm_yday)
    return true;

  if (!ofs_.is_open())
    return true;

  if (ofs_.tellp() > max_file_size_)
    return true;

  return false;
}
int RotateFileWriter::OpenNewFile() {
  if (ofs_.is_open()) {
    ofs_.flush();
    ofs_.clear();
    ofs_.close();
  }

  auto now = std::chrono::system_clock::now();
  time_t cnow = std::chrono::system_clock::to_time_t(now);
  localtime_r(&cnow, &t_);

  char name_buf[128];
  snprintf(name_buf, sizeof(name_buf), "%s_%04d%02d%02d_%02d%02d%02d", base_file_name_.c_str(),
           t_.tm_year + 1900, t_.tm_mon + 1, t_.tm_mday, t_.tm_hour, t_.tm_min, t_.tm_sec);

  ofs_.open(name_buf, std::ios::app);
  if (!ofs_.is_open()) {
    fprintf(stderr, "open file=%s failed.", name_buf);
    return -1;
  }

  return 0;
}

LogWriter GetRotateFileWriter(const std::map<std::string, std::string>& cfg) {
  auto path_itr = cfg.find("path");
  const std::string& path = (path_itr != cfg.end()) ? (path_itr->second) : "log";

  auto filename_itr = cfg.find("filename");
  const std::string& filename = (filename_itr != cfg.end()) ? (filename_itr->second) : "svr.log";

  const std::string& full_path = path + "/" + filename;

  fprintf(stderr, "init RotateFileWriter, file path:%s\n", full_path.c_str());

  std::filesystem::path log_path(path.c_str());
  if (!(std::filesystem::exists(log_path) && std::filesystem::is_directory(log_path))) {
    std::error_code errCode;
    if (!std::filesystem::create_directories(log_path, errCode)) {
      fprintf(stderr, "create_directories(%s) fail, error:%s\n", path.c_str(), errCode.message().c_str());
      return LogWriter();
    }
  }

  auto pw = std::make_shared<RotateFileWriter>();

  auto max_file_size_itr = cfg.find("max_file_size_m");
  if (max_file_size_itr != cfg.end()) {
    pw->SetMaxFileSize(std::atoll(max_file_size_itr->second.c_str()) * 1024 * 1024);
  }

  if (pw->Open(full_path) != 0) {
    fprintf(stderr, "RotateFileWriter.Open %s fail: %s\n", full_path.c_str(), strerror(errno));
    return LogWriter();
  }

  return [pw](const LogData& data) {
    pw->Write(data);
  };
}

}  // namespace ytlib
