/**
 * @file dynamic_lib.hpp
 * @brief 动态库加载工具
 * @note 跨平台的动态库加载工具。
 * 动态库运行时链接有两种方式：
 * 1、用的时候链接，用完即free：单纯使用DynamicLib类即可。
 * 2、类似于插件一样，一直保存着句柄，随时要用：使用DynamicLibContainer类。
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include <map>
#include <memory>
#include <string>

#include "error.hpp"

namespace ytlib {

#if defined(_WIN32)
  #if !defined(NOMINMAX) && defined(_MSC_VER)
    #define NOMINMAX  // required to stop windows.h messing up std::min
  #endif
  #include <windows.h>

typedef FARPROC SYMBOL_TYPE;
typedef HINSTANCE DYNLIB_HANDLE;
#else
  #include <dlfcn.h>
typedef void* SYMBOL_TYPE;
typedef void* DYNLIB_HANDLE;
#endif

/**
 * @brief 动态链接库封装类
 * 提供动态加载释放动态链接库并获取函数地址的能力
 */
class DynamicLib {
 public:
  DynamicLib() {}
  ~DynamicLib() { Free(); }

  operator bool() { return (NULL != hnd_); }

  ///获取动态链接库名称
  const std::string& GetLibName() const { return libname_; }

  ///根据名称加载动态链接库
  bool Load(const std::string& libname) {
    Free();

    libname_ = libname;

#if defined(_WIN32)
    if (libname_.size() <= 4 || libname_.substr(libname_.size() - 4) != ".dll")
      libname_ += ".dll";

    hnd_ = LoadLibraryEx(libname_.c_str(), NULL, 0);
#else
    if (libname_.substr(0, 3) != "lib")
      libname_ = "lib" + libname_;

    if (libname_.size() <= 3 || libname_.substr(libname_.size() - 3) != ".so")
      libname_ += ".so";

    hnd_ = dlopen(libname_.c_str(), RTLD_NOW | RTLD_GLOBAL);
#endif
    if (NULL == hnd_)
      fprintf(stderr, "load dynamic lib failed, lib path:%s, err:%s\n", libname_.c_str(), GetErr().c_str());

    return (NULL != hnd_);
  }

  ///释放动态链接库
  bool Free() {
    if (NULL == hnd_) return true;

    bool ret = false;
#if defined(_WIN32)
    ret = FreeLibrary(hnd_);
#else
    ret = (0 == dlclose(hnd_));
#endif
    if (!ret) {
      fprintf(stderr, "free dynamic lib failed, lib path:%s, err:%s\n", libname_.c_str(), GetErr().c_str());
      return false;
    }

    hnd_ = NULL;
    return true;
  }

  /// 获取函数地址
  SYMBOL_TYPE GetSymbol(const std::string& name) {
    RT_ASSERT(NULL != hnd_, "DynamicLib does not load any lib.");

#if defined(_WIN32)
    return GetProcAddress(hnd_, name.c_str());
#else
    return dlsym(hnd_, name.c_str());
#endif
  }

 private:
  std::string GetErr() const {
#if defined(_WIN32)
    LPTSTR lpMsgBuf;
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0,
        NULL);
    std::string ret = lpMsgBuf;
    LocalFree(lpMsgBuf);
    return ret;
#else
    return std::string(dlerror());
#endif
  }

  DYNLIB_HANDLE hnd_ = NULL;
  std::string libname_;
};

/**
 * @brief 动态链接库容器
 */
class DynamicLibContainer {
 public:
  static DynamicLibContainer& Ins() {
    static DynamicLibContainer instance;
    return instance;
  }

  ~DynamicLibContainer() {}

  std::shared_ptr<DynamicLib> LoadLib(const std::string& libname) {
    auto itr = lib_map_.find(libname);
    if (itr != lib_map_.end())
      return itr->second;

    std::shared_ptr<DynamicLib> lib = std::make_shared<DynamicLib>();
    if (lib->Load(libname)) {
      lib_map_.emplace(lib->GetLibName(), lib);
      return lib;
    }
    return std::shared_ptr<DynamicLib>();
  }

  std::shared_ptr<DynamicLib> GetLib(const std::string& libname) {
    auto itr = lib_map_.find(libname);
    if (itr != lib_map_.end())
      return itr->second;
    return std::shared_ptr<DynamicLib>();
  }

  bool RemoveLib(const std::string& libname) {
    auto itr = lib_map_.find(libname);
    if (itr == lib_map_.end()) return false;
    lib_map_.erase(itr);
    return true;
  }

 private:
  DynamicLibContainer() {}

  std::map<std::string, std::shared_ptr<DynamicLib> > lib_map_;
};

}  // namespace ytlib
