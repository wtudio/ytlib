#pragma once
#include <ytlib/SupportTools/DynamicLibrary.h>
#include <boost/serialization/singleton.hpp>

namespace wtlib
{
	//动态链接库容器
	class DynamicLibraryContainer
	{
	public:
		DynamicLibraryContainer(void) {}
		~DynamicLibraryContainer(void) {
			m_mapLibraries.clear();
		}

		//根据名称获取动态链接库对象
		std::shared_ptr<DynamicLibrary> GetLibrary(const tstring& libname) {
			std::map<tstring, std::shared_ptr<DynamicLibrary> >::iterator iter = m_mapLibraries.find(libname);
			if (iter == m_mapLibraries.end()) {
				std::shared_ptr<DynamicLibrary> pLibrary(new DynamicLibrary());
				if (pLibrary->Load(libname)) {
					m_mapLibraries.insert(std::make_pair(libname, pLibrary));
					return pLibrary;
				}
				else {
					pLibrary.reset();
					return pLibrary;
				}
			}
			else {
				return iter->second;
			}
		}
	private:
		std::map<tstring, std::shared_ptr<DynamicLibrary> > m_mapLibraries;
	};

	typedef boost::serialization::singleton<DynamicLibraryContainer> SingletonDynamicLibraryContainer;

#define GET_LIB(libname)	SingletonDynamicLibraryContainer::get_mutable_instance().GetLibrary(libname)


}