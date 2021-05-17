/**
 * @file DynamicLibraryContainer.h
 * @brief 动态库容器
 * @details 管理加载的动态库，形成动态库池
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/SupportTools/DynamicLibrary.h>

#include <boost/serialization/singleton.hpp>
#include <mutex>

namespace ytlib {
/**
 * @brief 动态链接库容器
 */
class DynamicLibraryContainer {
 public:
  DynamicLibraryContainer(void) {}
  ~DynamicLibraryContainer(void) {
    m_mapLibraries.clear();
  }

  ///根据名称获取动态链接库对象，并返回是否已经加载，true表示已经加载
  std::pair<std::shared_ptr<DynamicLibrary>, bool> GetLibrary(const tstring& libname) {
    std::lock_guard<std::mutex> lck(m_mapLibrariesMutex);
    std::map<tstring, std::shared_ptr<DynamicLibrary> >::iterator iter = m_mapLibraries.find(libname);
    if (iter == m_mapLibraries.end()) {
      std::shared_ptr<DynamicLibrary> pLibrary = std::make_shared<DynamicLibrary>();
      if (pLibrary->Load(libname)) {
        m_mapLibraries.insert(std::make_pair(libname, pLibrary));
        return std::pair<std::shared_ptr<DynamicLibrary>, bool>(pLibrary, false);
      } else {
        pLibrary.reset();
        return std::pair<std::shared_ptr<DynamicLibrary>, bool>(pLibrary, false);
      }
    } else {
      return std::pair<std::shared_ptr<DynamicLibrary>, bool>(iter->second, true);
    }
  }
  ///根据名称移除动态链接库对象
  bool RemoveLibrary(const tstring& libname) {
    std::lock_guard<std::mutex> lck(m_mapLibrariesMutex);
    std::map<tstring, std::shared_ptr<DynamicLibrary> >::iterator iter = m_mapLibraries.find(libname);
    if (iter == m_mapLibraries.end()) {
      //没有加载此lib
      return false;
    }
    m_mapLibraries.erase(iter);
    return true;
  }

 private:
  std::mutex m_mapLibrariesMutex;
  std::map<tstring, std::shared_ptr<DynamicLibrary> > m_mapLibraries;
};
///动态链接库容器的全局单例
typedef boost::serialization::singleton<DynamicLibraryContainer> SingletonDynamicLibraryContainer;

#define GET_LIB(libname) ytlib::SingletonDynamicLibraryContainer::get_mutable_instance().GetLibrary(libname)
#define REMOVE_LIB(libname) ytlib::SingletonDynamicLibraryContainer::get_mutable_instance().RemoveLibrary(libname)

}  // namespace ytlib