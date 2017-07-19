#pragma once

#include <ytlib/Common/Util.h>
#include <ytlib/Common/TString.h>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/xpressive/xpressive_dynamic.hpp>

namespace ytlib
{
	//封装了一些boost库不直接提供的。boost库有的直接调用boost库的
#if defined(UNICODE)
	typedef boost::filesystem::wpath tpath;
	typedef boost::xpressive::wsregex_compiler tsregex_compiler;
	typedef boost::filesystem::wrecursive_directory_iterator trecursive_directory_iterator;
#else
	typedef boost::filesystem::path tpath;
	typedef boost::xpressive::sregex_compiler tsregex_compiler;
	typedef boost::filesystem::recursive_directory_iterator trecursive_directory_iterator;
#endif


	//获得当前路径（可执行文件所在目录）
	tpath tGetCurrentPath(void);
	
	//获得绝对路径
	tpath tGetAbsolutePath(const tpath& p);

	//合并路径
	tpath tCombinePath(const tpath& p1, const tpath& p2);

}

