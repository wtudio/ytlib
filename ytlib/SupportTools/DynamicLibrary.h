#pragma once

#include <ytlib/Common/Util.h>
#include <ytlib/Common/TString.h>
#include <map>
#include <memory>

namespace ytlib
{
	/*
	动态库运行时链接有两种方式：
	1、用的时候链接，用完即free：单纯使用DynamicLibrary类即可
	2、类似于插件一样，一直保存着句柄，随时要用：使用DynamicLibraryContainer类
	*/
#if defined(_WIN32)
	typedef FARPROC SYMBOL_TYPE;
#else
	typedef void* SYMBOL_TYPE;
#endif

	//动态链接库封装类,提供动态加载释放动态链接库并获取函数地址的能力
	class DynamicLibrary
	{
	public:
		DynamicLibrary(void):m_hnd(NULL){}
		DynamicLibrary(const tstring& name) :m_hnd(NULL) {
			Load(name);
		}
		~DynamicLibrary(void) {
			Free();
		}
		operator bool() {
			return (NULL != m_hnd);
		}
		// 获取函数地址
		SYMBOL_TYPE GetSymbol(const tstring& name) {
			assert(NULL != m_hnd);

#if defined(_WIN32) 
			SYMBOL_TYPE symbol = ::GetProcAddress(m_hnd, T_TSTRING_TO_STRING(name).c_str());
#else
			SYMBOL_TYPE symbol = dlsym(m_hnd, T_TSTRING_TO_STRING(name).c_str());
#endif
			return symbol;
		}
		//获取动态链接库名称
		const tstring& GetLibraryName(void) const{ return m_libname; }

		//根据名称加载动态链接库
		bool Load(const tstring& libname) {
			m_libname = libname;
			Free();
#if defined(_WIN32)
			if ((m_libname.length()>4) && (m_libname.substr(m_libname.length() - 4, 4) != T_TEXT(".dll"))) {
				m_libname = m_libname + T_TEXT(".dll");
			}
			m_hnd = ::LoadLibrary(m_libname.c_str());
#else
			if (m_libname.substr(0, 3) != T_TEXT("lib")) {
				m_libname = T_TEXT("lib") + m_libname;
			}

			if (m_libname.substr(m_libname.length() - 3, 3) != T_TEXT(".so")) {
				m_libname = m_libname + T_TEXT(".so");
			}

			int32_t flags = RTLD_NOW | RTLD_GLOBAL;
			m_hnd = dlopen(T_TSTRING_TO_STRING(m_libname).c_str(), flags);
			if (NULL == m_hnd) {
				fprintf(stderr, "%s\n", dlerror());
			}
#endif
			return (NULL != m_hnd);
		}
		//释放动态链接库
		void Free(void) {
			if (NULL != m_hnd) {
#if defined(_WIN32)
				::FreeLibrary(m_hnd);
#else
				dlclose(m_hnd);
#endif
				m_hnd = NULL;
			}
		}

	private:
#if defined(_WIN32)
		HINSTANCE m_hnd;							// Windows平台的动态链接库句柄
#else
		void* m_hnd;								// 类Unix平台的动态链接库指针
#endif
		tstring m_libname;							// 动态链接库名称
	};

	
}


