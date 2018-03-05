#pragma once
#include <ytrpsf/Plugin_Bus_Interface.h>
#include <ytrpsf/Msg.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/SupportTools/Serialize.h>

namespace rpsf {
	//定义各种系统信息。各种类型的系统信息将储存数据的类序列化到buff中，收到时再反序列化回来
	//系统事件指各普通节点与中心节点进行的交互事件。普通节点之间不可能发送系统事件
	//例如订阅/取消订阅数据的事件，普通节点将此消息发送给中心节点，中心节点收到后再广播给所有节点
	//系统信息类型。最多256个
	enum SysMsgType {
		SYS_NEW_NODE_REG,//新节点向中心节点注册事件（com）（推送）
		SYS_NEW_NODE_REG_RESPONSE,//中心节点回复新节点注册事件（cen）（推送）
		SYS_NEW_NODE_ONLINE,//新节点上线事件，由中心节点发布，包含新节点的信息（订阅的系统事件）（cen）
		SYS_ALL_INFO,//更新整个网络当前状态，由中心节点发布，一般给供刚上线的节点更新全网信息（cen）
		SYS_SUB_DATAS,//订阅/取消订阅数据
		SYS_SUB_SERVICES,//注册/取消注册RPC
		SYS_SUB_SYSEVENT,//订阅/取消订阅系统事件
		SYS_HEART_BEAT,//心跳事件（cen）
		SYS_HEART_BEAT_RESPONSE,//心跳响应事件（com）
		SYS_COUNT	//计数，同时也是无效值
	};

	class rpsfSysMsg : public rpsfPackage {
	public:
		rpsfSysMsg(){}
		rpsfSysMsg(SysMsgType sysMsgType_):m_sysMsgType(sysMsgType_){}
		SysMsgType m_sysMsgType;

	};

	static rpsfPackagePtr setMsgToPackage(rpsfSysMsg& m_) {
		rpsfPackagePtr package = setBaseMsgToPackage(m_);
		package->obj.m_msgType = MsgType::RPSF_SYS;
		package->obj.m_handleType = HandleType::RPSF_SYNC;//默认为同步处理
		package->quick_data = ytlib::sharedBuf(1);
		package->quick_data.buf[0] = static_cast<uint8_t>(m_.m_sysMsgType);
		return std::move(package);
	}
	static void getMsgFromPackage(rpsfPackagePtr& package_, rpsfSysMsg& m_) {
		getBaseMsgFromPackage(package_, m_);
		m_.m_sysMsgType = static_cast<SysMsgType>(static_cast<uint8_t>(package_->quick_data.buf[0]));
	}
	static SysMsgType getSysMsgTypeFromPackage(const rpsfPackagePtr& package_) {
		return static_cast<SysMsgType>(static_cast<uint8_t>(package_->quick_data.buf[0]));
	}

#define SYSTAG "stg"
	template<class T>
	rpsfPackagePtr setSysMsgToPackage(T& m_, SysMsgType type_) {
		rpsfSysMsg sysMsg(type_);
		char* p; size_t len;
		SERIALIZE_INIT;
		SERIALIZE(m_, p, len);
		sysMsg.addData(SYSTAG, p, len);
		return setMsgToPackage(sysMsg);
	}
	template<class T>
	void getSysMsgFromPackage(rpsfPackagePtr& package_, T& m_) {
		std::map<std::string, ytlib::sharedBuf >::iterator itr = package_->map_datas.find(SYSTAG);
		DESERIALIZE_INIT;
		DESERIALIZE(m_, itr->second.buf.get(), itr->second.buf_size);
	}
	template<class T>
	void getSysMsgFromPackage(rpsfSysMsg& package_, T& m_) {
		std::map<std::string, ytlib::sharedBuf >::iterator itr = package_.m_mapDatas.find(SYSTAG);
		DESERIALIZE_INIT;
		DESERIALIZE(m_, itr->second.buf.get(), itr->second.buf_size);
	}

	//新节点向中心节点注册事件数据包
	class newNodeRegMsg {
		T_CLASS_SERIALIZE(&NodeId&SysMsg&NodeName)
	public:
		uint32_t NodeId;//节点id
		std::set<SysMsgType> SysMsg;//节点订阅的系统事件
		std::string NodeName;//节点名称（可作为节点补充信息）


	};
	//中心节点广播新节点上线事件数据包
	typedef newNodeRegMsg newNodeOnlineMsg;
	//全网信息数据包
	class allInfoMsg {
		T_CLASS_SERIALIZE(&mapSysMsgType2NodeId&mapDataNmae2NodeId&mapService2NodeId)
	public:
		std::map<SysMsgType, std::set<uint32_t> > mapSysMsgType2NodeId;
		std::map<std::string, std::set<uint32_t> > mapDataNmae2NodeId;
		std::map<std::string, std::set<uint32_t> > mapService2NodeId;
	};
	typedef allInfoMsg newNodeRegResponseMsg;
	//批量新订阅数据事件数据包
	class subscribeDatasInfoMsg {
		T_CLASS_SERIALIZE(&NodeId&DataNames&Operation)
	public:
		uint32_t NodeId;//节点id
		std::set<std::string> DataNames;//订阅的数据
		bool Operation;//操作：true订阅，false取消订阅
	};
	//批量新注册RPC事件数据包
	class servicesInfoMsg {
		T_CLASS_SERIALIZE(&NodeId&Services&Operation)
	public:
		uint32_t NodeId;//节点id
		std::set<std::string> Services;//提供的服务
		bool Operation;//操作：true提供服务，false取消提供服务
	};
	//批量订阅系统事件数据包
	class subscribeSysEventMsg {
		T_CLASS_SERIALIZE(&NodeId&SysMsg&Operation)
	public:
		uint32_t NodeId;//节点id
		std::set<SysMsgType> SysMsg;//节点订阅的系统事件
		bool Operation;//操作：true订阅，false取消订阅
	};
	//心跳监控事件数据包
	class heartBeatMsg {
		T_CLASS_SERIALIZE(&heartBeatIndex&heartBeatTime)
	public:
		uint32_t heartBeatIndex;
		uint64_t heartBeatTime;
	};
	//心跳监控事件响应数据包
	class heartBeatResponseMsg {
		T_CLASS_SERIALIZE(&nodeId&heartBeatIndex&heartBeatTime&cpuUsage&memUsage)
	public:
		uint32_t nodeId;
		uint32_t heartBeatIndex;
		uint64_t heartBeatTime;

		double cpuUsage;
		double memUsage;

	};

}


