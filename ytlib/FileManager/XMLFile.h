#pragma once
#include <ytlib/FileManager/FileBase.h>
#include <ytlib/SupportTools/XMLTools.h>

namespace wtlib
{
	//使用XML的文件类
	class XMLFile :public FileBase<tptree> {
	public:
		XMLFile() :FileBase() {}
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

}