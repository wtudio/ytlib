/**
 * @file XMLFile.h
 * @brief 使用XML的文件类
 * @details 使用XML的文件类，封装了xml解析
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "FileBase.h"
#include "XMLTools.h"

namespace ytlib {
/**
 * @brief 使用XML的文件类
 */
class XMLFile : public FileBase<tptree> {
 public:
  XMLFile() : FileBase() {}
  virtual ~XMLFile() {}

 protected:
  virtual bool GetFileObj() {
    if (!CreateFileObj()) return false;
    tpath path = tGetAbsolutePath(m_filepath);
    return realXml(path.string<tstring>(), *m_fileobj);
  }
  virtual bool SaveFileObj() {
    return writeXml(m_filepath, *m_fileobj);
  }
};

}  // namespace ytlib