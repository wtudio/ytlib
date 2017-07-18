#pragma once

#include <ytlib/Common/Util.h>
#include <ytlib/Common/TString.h>

namespace wtlib
{
	enum Error
	{
		ER_SUCCESS = 0,
		ER_NO_ERROR = 0,
		ER_ERROR_UNKNOWN_EXCEPTION,

		//序列化操作
		ER_ERROR_SERIALIZE_ERROR,
		ER_ERROR_DESERIALIZE_ERROR,

		//文件操作
		ER_ERROR_FILE_NOT_EXIST,
		ER_ERROR_INVALID_SAVE,
		ER_ERROR_INVALID_FILENAME,
		ER_ERROR_INVALID_FILE,
		ER_ERROR_NEW_FILE_FAILED,
		ER_ERROR_PARSE_FILE_FAILED,
		ER_ERROR_SAVE_FILE_FAILED,


		ER_ERROR_COUNT
	};

	const tchar* GetErrorMessage(Error err0);
	class Exception : public std::exception
	{
	public:
		Exception(Error err);
		Exception(const tstring& msg);
		virtual ~Exception(void) throw();
		virtual const char* what() const throw();
	protected:
		Error m_errorcode;
		tstring m_message;
	};

}
