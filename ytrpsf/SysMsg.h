#pragma once
#include <ytrpsf/Plugin_Bus_Interface.h>

namespace rpsf {
	//定义各种系统信息。各种类型的系统信息将储存数据的类序列化到buff中，收到时再反序列化回来

	//系统信息类型。最多256个
	enum SysMsgType {
		SYS_TEST1,
		SYS_TEST2,
		SYS_COUNT	//计数，同时也是无效值
	};

	class rpsfSysMsg : public rpsfPackage {
	public:
		rpsfSysMsg(){}
		rpsfSysMsg(SysMsgType sysMsgType_):m_sysMsgType(sysMsgType_){}
		SysMsgType m_sysMsgType;
	};



}


