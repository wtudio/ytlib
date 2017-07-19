#include <ytlib/Common/Error.h>

namespace ytlib
{
	static const tchar* gErrorMessages[ER_ERROR_COUNT] =
	{
		T_TEXT("操作successful"),
		T_TEXT("未知异常"),

		T_TEXT("序列化错误"),
		T_TEXT("反序列化错误"),

		T_TEXT("文件不存在"),
		T_TEXT("非法保存"),
		T_TEXT("非法文件名称"),
		T_TEXT("非法文件"),
		T_TEXT("初始化文件失败"),
		T_TEXT("解析文件失败"),
		T_TEXT("保存文件失败")
	};

	const tchar* GetErrorMessage(Error err0) {
		int32_t err = static_cast<int32_t>(err0);
		assert(err >= 0 && err < ER_ERROR_COUNT);
		const tchar* msg = gErrorMessages[err];
		assert(msg && msg[0]);
		return msg;
	}


	Exception::Exception(Error err)
		: m_message()
		, m_errorcode(err) {

	}

	Exception::Exception(const tstring& msg)
		: m_message(msg)
		, m_errorcode(ER_ERROR_UNKNOWN_EXCEPTION) {

	}

	Exception::~Exception(void) throw() {

	}

	const char* Exception::what() const throw() {
		if (m_message.empty()) {
			return T_TSTRING_TO_STRING(std::string(GetErrorMessage(m_errorcode))).c_str();
		}
		else {
			return T_TSTRING_TO_STRING(m_message).c_str();
		}
	}

}