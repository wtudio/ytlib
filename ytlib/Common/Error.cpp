#include <ytlib/Common/Error.h>

namespace wtlib
{
	static const tchar* gErrorMessages[ER_ERROR_COUNT] =
	{
		WT_TEXT("操作successful"),
		WT_TEXT("未知异常"),

		WT_TEXT("序列化错误"),
		WT_TEXT("反序列化错误"),

		WT_TEXT("文件不存在"),
		WT_TEXT("非法保存"),
		WT_TEXT("非法文件名称"),
		WT_TEXT("非法文件"),
		WT_TEXT("初始化文件失败"),
		WT_TEXT("解析文件失败"),
		WT_TEXT("保存文件失败")
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
			return WT_TSTRING_TO_STRING(std::string(GetErrorMessage(m_errorcode))).c_str();
		}
		else {
			return WT_TSTRING_TO_STRING(m_message).c_str();
		}
	}

}