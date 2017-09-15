#pragma once
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <boost/date_time/posix_time/posix_time.hpp>  
#include <ytrpsf/Plugin_Bus_Interface.h>

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
		T_CLASS_SERIALIZE(&m_srcAddr&m_msgType)
	public:
		rpsfMsg():m_delfiles(false){}

		uint32_t m_srcAddr;//消息源框架id
		uint8_t m_msgType;//消息类型。因为要支持序列化所以类型为int

		bool m_delfiles;//发送完成后是否删除文件
	};
	typedef ytlib::DataPackage<rpsfMsg> rpsfDataPackage;
	typedef std::shared_ptr<rpsfDataPackage> rpsfPackagePtr;

	//将各种类型的消息包打包为一个rpsfPackagePtr
	static rpsfPackagePtr setMsgToPackage(const rpsfRpcArgs& m_) {
		rpsfPackagePtr package = rpsfPackagePtr(new rpsfDataPackage());

		return package;
	}
	static rpsfPackagePtr setMsgToPackage(const rpsfData& m_) {
		rpsfPackagePtr package = rpsfPackagePtr(new rpsfDataPackage());

		return package;
	}
	static rpsfPackagePtr setMsgToPackage(const rpsfRpcResult& m_) {
		rpsfPackagePtr package = rpsfPackagePtr(new rpsfDataPackage());

		return package;
	}

	//将rpsfPackagePtr解包为各种类型的消息包
	static bool getMsgFromPackage(rpsfPackagePtr& package_, rpsfRpcArgs& m_) {

		return true;
	}
	static bool getMsgFromPackage(rpsfPackagePtr& package_, rpsfData& m_) {

		return true;
	}
	static bool getMsgFromPackage(rpsfPackagePtr& package_, rpsfRpcResult& m_) {

		return true;
	}
}


