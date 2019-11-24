/**
 * @file Error.h
 * @brief 基础错误、异常
 * @details ytlib中基础的错误、异常枚举
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/Common/Util.h>

namespace ytlib
{
	
	///ytlib中的一些错误枚举值
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
	///错误枚举值对应的错误信息
	static const char* gErrorMessages[ER_ERROR_COUNT] =
	{
		"successful.",
		"unknown error.",
		"serialize error.",
		"deserialize error.",
		"file not exist.",
		"invalid save.",
		"invalid file name.",
		"invalid file.",
		"initialize file failed.",
		"parse file failed.",
		"save file failed."
	};
	///根据err返回错误信息
	inline const char* GetErrorMessage(Error err0) {
		
		uint32_t err = static_cast<uint32_t>(err0);
		assert(err >= 0 && err < ER_ERROR_COUNT);
		const char* msg = gErrorMessages[err];
		assert(msg && msg[0]);
		return msg;
	}
	/**
	* @brief ytlib异常类
	* 重载了std::exception
	*/
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
