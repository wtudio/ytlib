#pragma once

#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <map>

#include "log.hpp"
#include "ytlib/string/string_util.hpp"

namespace ytlib {

enum class RotateType : int {
  RotateByTime,
  RotateByIndex,
};

class RotateFileWriter {
 public:
  RotateFileWriter() : rotate_type_(RotateType::RotateByTime){};
  ~RotateFileWriter() {
    if (ofs_.is_open()) {
      ofs_.flush();
      ofs_.clear();
      ofs_.close();
    }
  }

  void Write(const LogData& data) {
    if (CheckNeedRotate() && (0 != OpenNewFile()))
      return;

    ofs_ << "[" << data.time << "]["
         << LogLevelName::GetLogLevelName(data.lvl) << "]["
         << data.thread_id << "]";

    if (data.ctx) {
      for (auto& itr : *(data.ctx))
        ofs_ << "[" << itr.first << "=" << itr.second << "]";
    }

    ofs_ << data.msg << std::endl;
  }
  int Open(const std::string& file) {
    base_file_name_ = file;
    return OpenNewFile();
  }

  void SetMaxFileSize(size_t max_file_size) {
    // 至少1M
    if (max_file_size >= 1024 * 1024)
      max_file_size_ = max_file_size;
  }
  void SetMaxFileNum(uint32_t max_file_num) {
    max_file_num_ = max_file_num;
  }

  void SetRotateType(RotateType type) {
    rotate_type_ = type;
  }

 private:
  bool CheckNeedRotate() {
    if (rotate_type_ == RotateType::RotateByTime) {
      auto now = std::chrono::system_clock::now();
      time_t cnow = std::chrono::system_clock::to_time_t(now);
      struct tm now_tm;
#if defined(_WIN32)
      localtime_s(&now_tm, &cnow);
#else
      localtime_r(&cnow, &now_tm);
#endif

      if (now_tm.tm_hour != t_.tm_hour || now_tm.tm_yday != t_.tm_yday)
        return true;
    }

    if (!ofs_.is_open())
      return true;

    if (ofs_.tellp() > max_file_size_)
      return true;

    return false;
  }

  int OpenNewFile() {
    bool rename_flag = false;
    if (ofs_.is_open()) {
      if (ofs_.tellp() > max_file_size_)
        rename_flag = true;

      ofs_.flush();
      ofs_.clear();
      ofs_.close();
    }

    char name_buf[128];
    if (rotate_type_ == RotateType::RotateByIndex) {
      if (rename_flag) {
        std::filesystem::rename(base_file_name_, base_file_name_ + "_" + std::to_string(GetNextIndex()));
      }

      snprintf(name_buf, sizeof(name_buf), "%s", base_file_name_.c_str());
    } else {
      auto now = std::chrono::system_clock::now();
      time_t cnow = std::chrono::system_clock::to_time_t(now);
#if defined(_WIN32)
      localtime_s(&t_, &cnow);
#else
      localtime_r(&cnow, &t_);
#endif
      snprintf(name_buf, sizeof(name_buf), "%s_%04d%02d%02d_%02d%02d%02d", base_file_name_.c_str(),
               t_.tm_year + 1900, t_.tm_mon + 1, t_.tm_mday, t_.tm_hour, t_.tm_min, t_.tm_sec);
    }

    ofs_.open(name_buf, std::ios::app);
    if (!ofs_.is_open()) {
      fprintf(stderr, "open file %s failed.", name_buf);
      return -1;
    }

    CleanLogFile();

    return 0;
  }

  uint32_t GetNextIndex() {
    uint32_t idx = 1;
    std::filesystem::path log_dir = std::filesystem::path(base_file_name_).parent_path();
    std::filesystem::directory_iterator end_itr;
    for (std::filesystem::directory_iterator itr(log_dir); itr != end_itr; ++itr) {
      std::string tmp_logfile = itr->path().string();
      if (tmp_logfile.size() <= base_file_name_.size() + 1) continue;
      if (tmp_logfile.substr(0, base_file_name_.size() + 1) != (base_file_name_ + "_")) continue;
      std::string tmp_logfile_suffix = tmp_logfile.substr(base_file_name_.size() + 1);
      if (!IsDigitStr(tmp_logfile_suffix)) continue;
      uint32_t cur_idx = atoi(tmp_logfile_suffix.c_str());
      if (cur_idx >= idx) idx = cur_idx + 1;
    }

    return idx;
  }

  void CleanLogFile() {
    if (max_file_num_ == 0) return;

    std::filesystem::path log_dir = std::filesystem::path(base_file_name_).parent_path();

    if (rotate_type_ == RotateType::RotateByIndex) {
      std::map<uint32_t, std::string> log_files;

      std::filesystem::directory_iterator end_itr;
      for (std::filesystem::directory_iterator itr(log_dir); itr != end_itr; ++itr) {
        std::string tmp_logfile = itr->path().string();
        if (tmp_logfile.size() <= base_file_name_.size() + 1) continue;
        if (tmp_logfile.substr(0, base_file_name_.size() + 1) != (base_file_name_ + "_")) continue;
        std::string tmp_logfile_suffix = tmp_logfile.substr(base_file_name_.size() + 1);
        if (!IsDigitStr(tmp_logfile_suffix)) continue;
        uint32_t cur_idx = atoi(tmp_logfile_suffix.c_str());
        log_files.emplace(cur_idx, tmp_logfile);
      }

      if (log_files.size() <= max_file_num_) return;

      uint32_t del_num = log_files.size() - max_file_num_;
      auto itr = log_files.begin();
      do {
        std::filesystem::remove(itr->second);
      } while (--del_num && itr != log_files.end());

    } else {
      // rotate_type_ == RotateType::RotateByTime
      std::map<std::string, std::string> log_files;

      std::filesystem::directory_iterator end_itr;
      for (std::filesystem::directory_iterator itr(log_dir); itr != end_itr; ++itr) {
        // %s_%04d%02d%02d_%02d%02d%02d
        std::string tmp_logfile = itr->path().string();
        if (tmp_logfile.size() <= base_file_name_.size() + 1) continue;
        if (tmp_logfile.substr(0, base_file_name_.size() + 1) != (base_file_name_ + "_")) continue;
        std::string tmp_logfile_suffix = tmp_logfile.substr(base_file_name_.size() + 1);
        if (tmp_logfile_suffix.size() != 15) continue;
        if (tmp_logfile_suffix[8] != '_') continue;
        if (!IsDigitStr(tmp_logfile_suffix.substr(0, 8))) continue;
        if (!IsDigitStr(tmp_logfile_suffix.substr(9))) continue;
        log_files.emplace(tmp_logfile_suffix, tmp_logfile);
      }

      if (log_files.size() <= max_file_num_) return;

      uint32_t del_num = log_files.size() - max_file_num_;
      auto itr = log_files.begin();
      do {
        std::filesystem::remove(itr->second);
      } while (--del_num && itr != log_files.end());
    }
  }

 private:
  RotateType rotate_type_;
  std::string base_file_name_;  // 基础文件路径
  std::ofstream ofs_;

  struct tm t_;  // 当前文件创建时间，仅RotateByTime时使用

  std::ofstream::pos_type max_file_size_ = 16 * 1024 * 1024;  // 最大日志文件大小，默认16M
  uint32_t max_file_num_ = 0;                                 // 最大日志文件数量，0代表无限
};

inline LogWriter GetRotateFileWriter(const std::map<std::string, std::string>& cfg) {
  auto path_itr = cfg.find("path");
  const std::string& path = (path_itr != cfg.end()) ? (path_itr->second) : "log";

  auto filename_itr = cfg.find("filename");
  const std::string& filename = (filename_itr != cfg.end()) ? (filename_itr->second) : "svr.log";

  std::filesystem::path log_path(path.c_str());
  const std::string& full_path = (log_path / filename).string();

  fprintf(stderr, "init RotateFileWriter, file path:%s\n", full_path.c_str());

  if (!(std::filesystem::exists(log_path) && std::filesystem::is_directory(log_path))) {
    std::error_code errCode;
    if (!std::filesystem::create_directories(log_path, errCode)) {
      fprintf(stderr, "create_directories(%s) fail, error:%s\n", path.c_str(), errCode.message().c_str());
      return LogWriter();
    }
  }

  auto pw = std::make_shared<RotateFileWriter>();

  auto rotate_type_itr = cfg.find("rotate_type");
  const std::string& rotate_type = (rotate_type_itr != cfg.end()) ? (rotate_type_itr->second) : "time";
  if (rotate_type == "index") {
    pw->SetRotateType(RotateType::RotateByIndex);
  } else {
    pw->SetRotateType(RotateType::RotateByTime);
  }

  auto max_file_size_itr = cfg.find("max_file_size_m");
  if (max_file_size_itr != cfg.end()) {
    pw->SetMaxFileSize(std::atoll(max_file_size_itr->second.c_str()) * 1024 * 1024);
  }

  auto max_file_num_itr = cfg.find("max_file_num");
  if (max_file_num_itr != cfg.end()) {
    pw->SetMaxFileNum(std::atoi(max_file_num_itr->second.c_str()));
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
