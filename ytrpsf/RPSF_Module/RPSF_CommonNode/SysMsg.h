#pragma once
#include <ytrpsf/RPSF_Module/RPSF_CommonNode/Msg.h>

namespace rpsf {
	//用于系统消息。不提供给插件调用，一般用于传递各种结构体
	class rpsfSys {
		T_CLASS_SERIALIZE(&m_sysMsgType)
	public:
		//系统消息类型
		enum {
			RPSF_SYS_NEWNODE
		};
		rpsfSys() :m_msgType(MsgType::RPSF_SYS | handleType::RPSF_EMCY) {

		}

		uint8_t m_sysMsgType;//消息类型


		std::map<std::string, ytlib::shared_buf> map_datas;//数据,最大支持255个

		uint8_t m_msgType;
	};
}


