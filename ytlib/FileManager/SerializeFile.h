#pragma once

#include <ytlib/FileManager/FileBase.h>
#include <ytlib/SupportTools/Serialize.h>

namespace ytlib
{
	//使用boost序列化的文件类
	//T需要支持序列化
	template <class T>
	class SerializeFile :public FileBase<T> {
	public:
		SerializeFile():FileBase<T>(){}
		virtual ~SerializeFile() {}
	protected:
		
		virtual bool GetFileObj() {
			if (!FileBase<T>::CreateFileObj()) return false;
			tpath path = tGetAbsolutePath(FileBase<T>::m_filepath);
			return Deserialize_f(*FileBase<T>::m_fileobj, path.string<tstring>());
		}
		virtual bool SaveFileObj() {
			return Serialize_f(*FileBase<T>::m_fileobj, FileBase<T>::m_filepath);
		}
		
	};
}
