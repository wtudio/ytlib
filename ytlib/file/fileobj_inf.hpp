/**
 * @file fileobj_inf.hpp
 * @brief 文件基础类
 * @note 文件基础类，制定了文件相关操作的接口
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <filesystem>
#include <memory>

namespace ytlib {

/**
 * @brief 文件基础类
 * @note 制定了文件相关操作的接口，包括Open、New、Save
 * @tparam T 文件可转换的结构体
 */
template <class T>
class FileObj {
 public:
  /// 构造函数
  FileObj() : obj_ptr_(std::make_shared<T>()) {}

  /// 析构函数
  virtual ~FileObj() = default;

  /**
   * @brief 打开并解析文件内容
   * @note 打开并解析文件内容到obj，同时设置path
   * @param[in] path 文件路径
   */
  void OpenFile(const std::string& path) {
    SetFilePath(path);
    OpenFile();
  }

  /**
   * @brief 打开并解析文件内容
   * @note 打开并解析文件内容到obj，使用设置好的path
   */
  void OpenFile() {
    if (filepath_.empty() || std::filesystem::status(filepath_).type() != std::filesystem::file_type::regular)
      throw std::logic_error("File path invalid.");

    obj_ptr_.reset();

    if (!ParseFileObj())
      throw std::logic_error("Parse file failed.");
  }

  /**
   * @brief 初始化内容结构体
   * @note 初始化内容结构体，同时会设置path，如果已经打开并解析了某个文件，则不会保存
   * @param[in] path 文件路径
   */
  void NewFile(const std::string& path) {
    SetFilePath(path);
    NewFile();
  }

  /**
   * @brief 初始化内容结构体
   * @note 初始化内容结构体，如果已经打开并解析了某个文件，则不会保存
   */
  void NewFile() {
    obj_ptr_.reset();

    if (!NewFileObj())
      throw std::logic_error("New file failed.");
  }

  /**
   * @brief 保存内容结构体
   * @note 保存内容结构体，同时会设置path
   * @param[in] path 文件路径
   */
  void SaveFile(const std::string& path) {
    SetFilePath(path);
    SaveFile();
  }

  /**
   * @brief 保存内容结构体
   * @note 保存内容结构体，使用设置好的path
   */
  void SaveFile() {
    if (filepath_.empty())
      throw std::logic_error("File path is empty.");

    const auto& parent_path = filepath_.parent_path();
    if (std::filesystem::status(parent_path).type() != std::filesystem::file_type::directory &&
        !std::filesystem::create_directories(parent_path)) {
      throw std::logic_error("Create dir failed.");
    }

    if (!SaveFileObj())
      throw std::logic_error("Save file failed.");
  }

  /**
   * @brief 获取文件结构体智能指针
   *
   * @return const std::shared_ptr<T>& 文件结构体智能指针
   */
  const std::shared_ptr<T>& GetObjPtr() const {
    return obj_ptr_;
  }

  /**
   * @brief 获取绝对路径
   *
   * @return const std::filesystem::path& 文件绝对路径
   */
  const std::filesystem::path& GetFilePath() const {
    return filepath_;
  }

  /**
   * @brief 设置路径
   *
   * @param[in] path 文件路径
   */
  void SetFilePath(const std::string& path) {
    const std::filesystem::path& p = std::filesystem::absolute(path);
    if (!CheckFileName(p.string()))
      throw std::logic_error("Invalid file path.");

    filepath_ = p;
  }

 protected:
  /**
   * @brief 检查文件名称正确性
   * @note 子类可选实现。一般就是检查后缀名
   * @param[in] filename 文件全路径
   * @return true 合法
   * @return false 不合法
   */
  virtual bool CheckFileName(const std::string& filename) const {
    return !filename.empty();
  }

  /**
   * @brief 新建一个文件内容结构体
   * @note 子类可选实现。做一些内容结构体初始化的工作
   * @return true 成功
   * @return false 不成功
   */
  virtual bool NewFileObj() {
    obj_ptr_ = std::make_shared<T>();
    return true;
  }

  /**
   * @brief 从打开的文件中解析获取文件内容结构体
   * @note 子类必选实现
   * @return true 成功
   * @return false 不成功
   */
  virtual bool ParseFileObj() = 0;

  /**
   * @brief 将当前的文件内容结构体保存为文件
   * @note 子类必选实现
   * @return true 成功
   * @return false 不成功
   */
  virtual bool SaveFileObj() = 0;

 protected:
  std::shared_ptr<T> obj_ptr_;      ///< 文件内容解析后的结构体
  std::filesystem::path filepath_;  ///< 文件绝对路径
};
}  // namespace ytlib
