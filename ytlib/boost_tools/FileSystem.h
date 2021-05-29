/**
 * @file FileSystem.h
 * @brief 文件相关操作
 * @details 基于boost库的一些基础文件操作。封装了一些boost库不直接提供的。boost库有的直接调用boost库的
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "TString.h"
#include "Util.h"

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>

namespace ytlib {
#if defined(UNICODE)
typedef boost::filesystem::wpath tpath;
typedef boost::xpressive::wsregex_compiler tsregex_compiler;
typedef boost::filesystem::wrecursive_directory_iterator trecursive_directory_iterator;
  #define T_PATH_TO_TSTRING(PATH) PATH.wstring()
#else
typedef boost::filesystem::path tpath;
typedef boost::xpressive::sregex_compiler tsregex_compiler;
typedef boost::filesystem::recursive_directory_iterator trecursive_directory_iterator;
  #define T_PATH_TO_TSTRING(PATH) PATH.string()
#endif

///获得当前路径（可执行文件所在目录）
inline tpath tGetCurrentPath(void) {
#if defined(_WIN32)

  #include <shlwapi.h>
  #pragma comment(lib, "shlwapi.lib")

  TCHAR szBuffer[MAX_PATH] = {0};
  ::GetModuleFileName(NULL, szBuffer, MAX_PATH);
  return tpath(szBuffer).parent_path();
#else
  return boost::filesystem::initial_path<tpath>();
#endif
}

///获得绝对路径
inline tpath tGetAbsolutePath(const tpath& p) {
  return (p.is_absolute()) ? p : (tGetCurrentPath() / p);
}
///获得文件所在目录
inline tpath tGetDirectory(const tpath& p) {
  return tGetAbsolutePath(p).parent_path();
}

}  // namespace ytlib
