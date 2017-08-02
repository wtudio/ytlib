#pragma once

#include <ytlib/Common/Util.h>
#include <ytlib/Common/TString.h>

namespace ytlib
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

	static const tchar* GetErrorMessage(Error err0) {
		static const tchar* gErrorMessages[ER_ERROR_COUNT] =
		{
			T_TEXT("操作successful."),
			T_TEXT("未知异常."),
			T_TEXT("序列化错误."),
			T_TEXT("反序列化错误."),
			T_TEXT("文件不存在."),
			T_TEXT("非法保存."),
			T_TEXT("非法文件名称."),
			T_TEXT("非法文件."),
			T_TEXT("初始化文件失败."),
			T_TEXT("解析文件失败."),
			T_TEXT("保存文件失败.")
		};

		int32_t err = static_cast<int32_t>(err0);
		assert(err >= 0 && err < ER_ERROR_COUNT);
		const tchar* msg = gErrorMessages[err];
		assert(msg && msg[0]);
		return msg;
	}
	class Exception : public std::exception
	{
	public:
		

		Exception(Error err)
			: m_message()
			, m_errorcode(err) {

		}
		Exception(const tstring& msg)
			: m_message(msg)
			, m_errorcode(ER_ERROR_UNKNOWN_EXCEPTION) {

		}
		virtual ~Exception(void) throw(){}
		virtual const char* what() const throw() {
			if (m_message.empty()) {
				return T_TSTRING_TO_STRING(tstring(GetErrorMessage(m_errorcode))).c_str();
			}
			else {
				return T_TSTRING_TO_STRING(m_message).c_str();
			}
		}
	protected:
		Error m_errorcode;
		tstring m_message;
	};

}
