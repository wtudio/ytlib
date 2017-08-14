#pragma once
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <boost/date_time/posix_time/posix_time.hpp>  

namespace rpsf {

	//本文件中实现各种数据包
	enum MsgType {
		RPSF_DATA = 0,
		RPSF_EVENT = 1 * 16,
		RPSF_RPC = 2 * 16,
		RPSF_RRPC = 3 * 16,
		RPSF_SYS = 4 * 16
	};

	//最底层数据包格式。快速数据： event：数据，RRPC：错误信息
	class rpsfMsg {
		T_CLASS_SERIALIZE(&m_srcAddr&m_msgType&s1&s2&i1&i2)
	public:
		rpsfMsg():m_delfiles(false){}

		uint32_t m_srcAddr;//消息源框架id
		uint8_t m_msgType;//消息类型。因为要支持序列化所以类型为int
		//一些快速访问数据可以放在这
		std::string s1;//data\event：数据名称，RPC：service名称
		std::string s2;//data\event：发送者，RPC：fun名称
		uint32_t i1;//event：时间高32位，RPC\RRPC：rpcID 
		uint32_t i2;//event：时间低32位，RPC：timeout，RRPC错误码

		bool m_delfiles;//发送完成后是否删除文件
	};
	typedef ytlib::DataPackage<rpsfMsg> rpsfDataPackage;
	typedef std::shared_ptr<rpsfDataPackage> rpsfPackagePtr;

}


