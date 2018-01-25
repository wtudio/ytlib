#pragma once
#include <ytrpsf/Msg.h>
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
			m_netAdapter = std::make_shared<ytlib::TcpNetAdapter<rpsfMsg> >(thisnode_.NodeId, thisnode_.NodePort, std::bind(&Bus::on_RecvCallBack, this, std::placeholders::_1), recvpath, sendpath);

			

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




			ytlib::StopNetLog();

			m_bRunning = false;

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
		void on_RecvCallBack(rpsfPackagePtr& pmsg) {
			switch (pmsg->obj.m_handleType) {
			case HandleType::RPSF_SYNC :
				rpsfMsgHandler(pmsg);
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
				//如果是本地的，找出目的地，如果是其他节点，则通过网络发送
				pmsg->obj.m_srcAddr = m_NodeId;



				switch (pmsg->obj.m_msgType) {
				case MsgType::RPSF_SYS:

					break;
				case MsgType::RPSF_DATA: {
		
					break;
				}
				case MsgType::RPSF_RPC: {
	
					break;
				}
				case MsgType::RPSF_RRPC: {
		
					break;
				}
				default:
					break;
				}

			}

			//如果目的地是本地节点，或者消息是从网络上收到的，则进行处理


			switch (pmsg->obj.m_msgType) {
			case MsgType::RPSF_SYS:

				break;
			case MsgType::RPSF_DATA: {
				rpsfData data;
				getMsgFromPackage(pmsg, data);

				break;
			}	
			case MsgType::RPSF_RPC: {
				rpsfRpcArgs rpcArgs;
				getMsgFromPackage(pmsg, rpcArgs);

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

		//所有成员都只能用智能指针了
		std::shared_ptr<ytlib::TcpNetAdapter<rpsfMsg> >  m_netAdapter;//网络适配器
		std::shared_ptr<ytlib::ChannelBase<rpsfPackagePtr> > m_orderChannel;//有序通道
		std::shared_ptr<ytlib::ChannelBase<rpsfPackagePtr> > m_unorderChannel;//无序通道

		bool m_bInit;
		bool m_bRunning;

		uint32_t m_NodeId;
	};


}