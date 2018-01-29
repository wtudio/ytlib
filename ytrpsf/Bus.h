#pragma once
#include <ytrpsf/Msg.h>
#include <ytrpsf/RpsfCfgFile.h>
#include <ytlib/LogService/Logger.h>
#include <ytlib/SupportTools/ChannelBase.h>


namespace rpsf {

	static const char* busErrMsg[RPSF_ERR_COUNT] =
	{
		"successful.",
		"file not exist.",
		"service not exist."
		"RPC timeout.",
		"RPC failed.",
		"subscribe data failed",
		"unsubscribe data failed",
		"subscribe service failed",
		"unsubscribe service failed"
	};

	//总线。一些通用的操作放在此处完成
	class Bus : public IBus {
	public:
		
		Bus() :m_bInit(false), m_bRunning(false) {}
		virtual ~Bus() { Stop(); }

		virtual bool Init(const RpsfNode& thisnode_) {
			m_NodeId = thisnode_.NodeId;
			//初始化日志
			if (thisnode_.UseNetLog) {
				ytlib::InitNetLog(thisnode_.NodeId, ytlib::TcpEp(boost::asio::ip::address::from_string(thisnode_.LogServerIp), thisnode_.LogServerPort));
			}

			//初始化各个表


			//设置通道
			m_orderChannel = std::make_shared<ytlib::ChannelBase<rpsfPackagePtr> >(std::bind(&Bus::rpsfMsgHandler, this, std::placeholders::_1));
			m_unorderChannel = std::make_shared<ytlib::ChannelBase<rpsfPackagePtr> >(std::bind(&Bus::rpsfMsgHandler, this, std::placeholders::_1), 5);
		

			//设置文件收发路径
			ytlib::tstring nodename(T_STRING_TO_TSTRING(thisnode_.NodeName));
			ytlib::tstring recvpath(nodename + T_TEXT("/recv"));
			ytlib::tstring sendpath(nodename + T_TEXT("/send"));
			std::map<ytlib::tstring, ytlib::tstring>::const_iterator itr = thisnode_.NodeSettings.find(T_TEXT("recvpath"));
			if (itr != thisnode_.NodeSettings.end()) recvpath = itr->second;
			itr = thisnode_.NodeSettings.find(T_TEXT("sendpath"));
			if (itr != thisnode_.NodeSettings.end()) sendpath = itr->second;
			//最后才能开启网络适配器监听
			m_netAdapter = std::make_shared<ytlib::TcpNetAdapter<rpsfMsg> >(thisnode_.NodeId, thisnode_.NodePort, std::bind(&Bus::rpsfMsgClassifier, this, std::placeholders::_1), recvpath, sendpath);

			

			return true;

		}

		virtual bool Start() {
			if (!m_bInit)return false;


			if (!(m_netAdapter->start())) return false;


			m_bRunning = true;
			return m_bRunning;
		}

		virtual bool Stop() {
			if (!m_bRunning) return true;//已经停止了
			m_bRunning = false;


			m_orderChannel->Stop();
			m_unorderChannel->Stop();
			m_netAdapter->stop();
			ytlib::StopNetLog();

			return !m_bRunning;
		}

		//接口：bus-plugin
		BusErr SubscribeData(const IPlugin* pPlugin_, const std::string& dataNames_) {

		}
		BusErr UnsubscribeData(const IPlugin* pPlugin_, const std::string& dataName_) {

		}
		std::set<std::string> GetSubscribeDataList(const IPlugin* pPlugin_) {

		}

		BusErr SubscribeService(const IPlugin* pPlugin_, const std::string& service_, const std::string& remark_ = "") {

		}
		BusErr UnsubscribeService(const IPlugin* pPlugin_, const std::string& service_) {

		}
		std::map<std::string, std::string> GetSubscribeServiceList(const IPlugin* pPlugin_) {

		}
		rpsfRpcResult Invoke(rpsfRpcArgs& callArgs_, uint32_t timeout = 0) {

		}
		BusErr PublishData(rpsfData& data_) {

		}

		const char* getBusErrMsg(BusErr err) {
			return busErrMsg[err];
		}
		
	protected:

		//从网络收到消息的回调
		void rpsfMsgClassifier(rpsfPackagePtr& pmsg) {
			switch (pmsg->obj.m_handleType) {
			case HandleType::RPSF_SYNC :
				rpsfMsgHandlerLocal(pmsg);
				break;
			case HandleType::RPSF_ORDER:
				m_orderChannel->Add(pmsg);
				break;
			case HandleType::RPSF_UNORDER:
				m_unorderChannel->Add(pmsg);
				break;
			default:
				break;
			}

		}

		//处理信息
		void rpsfMsgHandler(rpsfPackagePtr& pmsg) {
			//先判断是否是本地发出的
			if (pmsg->obj.m_srcAddr == 0) {
				pmsg->obj.m_srcAddr = m_NodeId;
				//如果是本地的，找出目的地，如果目的地是其他节点，则通过网络发送
				std::set<uint32_t> dst;

				//根据数据类型，从不同的表里找目的地
				switch (pmsg->obj.m_msgType) {
				case MsgType::RPSF_SYS: {
					//找有哪些节点订阅了此系统事件
					std::shared_lock<std::shared_mutex> lck(m_mapSysMsgType2NodeIdMutex);
					std::map<SysMsgType, std::set<uint32_t> >::const_iterator itr =	m_mapSysMsgType2NodeId.find(getSysMsgTypeFromPackage(pmsg));
					if (itr != m_mapSysMsgType2NodeId.end()) dst = itr->second;
					break;
				}
				case MsgType::RPSF_DATA: {
					//找有哪些节点订阅了此数据
					std::shared_lock<std::shared_mutex> lck(m_mapDataNmae2NodeIdMutex);
					std::map<std::string, std::set<uint32_t> >::const_iterator itr = m_mapDataNmae2NodeId.find(getDataNameFromPackage(pmsg));
					if (itr != m_mapDataNmae2NodeId.end()) dst = itr->second;
					break;
				}
				case MsgType::RPSF_RPC: {
					//找有哪些节点提供此RPC服务
					std::shared_lock<std::shared_mutex> lck(m_mapService2NodeIdMutex);
					std::map<std::string, std::set<uint32_t> >::const_iterator itr = m_mapService2NodeId.find(getServiceFromPackage(pmsg));
					if (itr != m_mapService2NodeId.end()) dst = itr->second;
					break;
				}
				case MsgType::RPSF_RRPC: {
					//找这个RPC服务是从哪个节点发出来的
					dst.insert(getRrpcDstFromPackage(pmsg));
					break;
				}
				default:
					break;
				}

				//如果目的地是本地，则进行处理
				std::set<uint32_t>::const_iterator itr = dst.find(m_NodeId);
				if (itr != dst.end()) {
					rpsfMsgHandlerLocal(pmsg);
					dst.erase(itr);
				}

				//先进行本地处理再发送到网络
				if (dst.size() > 0)	m_netAdapter->Send(pmsg, dst, pmsg->obj.m_delfiles);
			}
			else {
				//如果是从网络收到的
				rpsfMsgHandlerLocal(pmsg);
			}
		}

		void rpsfMsgHandlerLocal(rpsfPackagePtr& pmsg){
			//如果目的地是本地节点，或者消息是从网络上收到的（目的地肯定是本地），则进行处理
			switch (pmsg->obj.m_msgType) {
			case MsgType::RPSF_SYS: {
				rpsfSysMsg sysMsg;
				getMsgFromPackage(pmsg, sysMsg);
				rpsfSysHandler(sysMsg);
				break;
			}
			case MsgType::RPSF_DATA: {
				rpsfData data;
				getMsgFromPackage(pmsg, data);
				//找到本地对应的插件进行派发

				break;
			}	
			case MsgType::RPSF_RPC: {
				rpsfRpcArgs rpcArgs;
				getMsgFromPackage(pmsg, rpcArgs);
				//找到本地的插件进行调用

				//将结果打包并交由分类器处理

				break;
			}
			case MsgType::RPSF_RRPC: {
				rpsfRpcResult rpcResult;
				getMsgFromPackage(pmsg, rpcResult);

				break;
			}
			default:
				break;
			}
			
		}

		//系统信息处理。通用的一些系统事件在此次处理，其他的交给上层
		void rpsfSysHandler(const rpsfSysMsg& m_) {
			switch (m_.m_sysMsgType) {
			case SysMsgType::SYS_TEST1: {


				break;
			}
			case SysMsgType::SYS_TEST2: {


				break;
			}
			case SysMsgType::SYS_COUNT: {

				break;
			}
			default:

				break;
			}

		}
		


		//所有成员都只能用智能指针了
		std::shared_ptr<ytlib::TcpNetAdapter<rpsfMsg> >  m_netAdapter;//网络适配器
		std::shared_ptr<ytlib::ChannelBase<rpsfPackagePtr> > m_orderChannel;//有序通道
		std::shared_ptr<ytlib::ChannelBase<rpsfPackagePtr> > m_unorderChannel;//无序通道

		bool m_bInit;
		bool m_bRunning;

		uint32_t m_NodeId;

		//系统事件与节点id的表
		std::shared_mutex m_mapSysMsgType2NodeIdMutex;
		std::map<SysMsgType, std::set<uint32_t> > m_mapSysMsgType2NodeId;
		//数据名称与节点id的表
		std::shared_mutex m_mapDataNmae2NodeIdMutex;
		std::map<std::string, std::set<uint32_t> > m_mapDataNmae2NodeId;
		//服务名称与节点id的表
		std::shared_mutex m_mapService2NodeIdMutex;
		std::map<std::string, std::set<uint32_t> > m_mapService2NodeId;



	};


}