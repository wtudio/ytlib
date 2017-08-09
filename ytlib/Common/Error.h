#pragma once

#include <ytlib/Common/Util.h>

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
	static const char* gErrorMessages[ER_ERROR_COUNT] =
	{
		"操作successful.",
		"未知异常.",
		"序列化错误.",
		"反序列化错误.",
		"文件不存在.",
		"非法保存.",
		"非法文件名称.",
		"非法文件.",
		"初始化文件失败.",
		"解析文件失败.",
		"保存文件失败."
	};

	static const char* GetErrorMessage(Error err0) {
		
		uint32_t err = static_cast<uint32_t>(err0);
		assert(err >= 0 && err < ER_ERROR_COUNT);
		const char* msg = gErrorMessages[err];
		assert(msg && msg[0]);
		return msg;
	}
	class Exception : public std::exception	{
	public:

		Exception(Error err): m_message(), m_errorcode(err) {

		}
		Exception(const std::string& msg): m_message(msg), m_errorcode(ER_ERROR_UNKNOWN_EXCEPTION) {

		}
		virtual ~Exception(void) throw(){}
		virtual const char* what() const throw() {
			if (m_message.empty()) {
				return GetErrorMessage(m_errorcode);
			}
			else {
				return m_message.c_str();
			}
		}
	protected:
		Error m_errorcode;
		std::string m_message;
	};

}
