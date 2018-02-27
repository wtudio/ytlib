#pragma once
#include <ytrpsf/SysMsg.h>
#include <ytrpsf/RpsfCfgFile.h>
#include <ytlib/LogService/Logger.h>
#include <ytlib/SupportTools/ChannelBase.h>
#include <ytlib/SupportTools/DynamicLibraryContainer.h>


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
		
		Bus() :m_bRunning(false){}
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

			if (!(m_netAdapter->start())) return false;


			return true;

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

			return BusErr::RPSF_NO_ERROR;
		}
		BusErr UnsubscribeData(const IPlugin* pPlugin_, const std::string& dataName_) {

			return BusErr::RPSF_NO_ERROR;
		}
		std::set<std::string> GetSubscribeDataList(const IPlugin* pPlugin_) {

			return std::set<std::string>();
		}

		BusErr SubscribeService(const IPlugin* pPlugin_, const std::string& service_, const std::string& remark_ = "") {

			return BusErr::RPSF_NO_ERROR;
		}
		BusErr UnsubscribeService(const IPlugin* pPlugin_, const std::string& service_) {

			return BusErr::RPSF_NO_ERROR;
		}
		std::map<std::string, std::string> GetSubscribeServiceList(const IPlugin* pPlugin_) {

			return std::map<std::string, std::string>();
		}

		rpsfRpcResult Invoke(rpsfRpcArgs& callArgs_, uint32_t timeout = 0) {

			return rpsfRpcResult();
		}

		BusErr PublishData(rpsfData& data_) {
			rpsfMsgClassifier(setMsgToPackage(data_));

			return BusErr::RPSF_NO_ERROR;
		}

		const char* getBusErrMsg(BusErr err) {
			return busErrMsg[err];
		}
		
	protected:
		//本地订阅系统事件
		virtual void SubscribeSysEvent(const std::set<SysMsgType>& sysEvents_) {
			std::unique_lock<std::shared_mutex> lck(m_mapSysMsgType2NodeIdMutex);
			for (std::set<SysMsgType>::const_iterator itr = sysEvents_.begin(); itr != sysEvents_.end(); ++itr) {
				m_mapSysMsgType2NodeId[*itr].insert(m_NodeId);
			}

		}
		//本地取消订阅系统事件
		virtual void UnsubscribeSysEvent(const std::set<SysMsgType>& sysEvents_) {
			std::unique_lock<std::shared_mutex> lck(m_mapSysMsgType2NodeIdMutex);
			for (std::set<SysMsgType>::const_iterator itr = sysEvents_.begin(); itr != sysEvents_.end(); ++itr) {
				std::map<SysMsgType, std::set<uint32_t> >::iterator itr2 = m_mapSysMsgType2NodeId.find(*itr);
				if (itr2 == m_mapSysMsgType2NodeId.end()) continue;
				std::set<uint32_t>::iterator itr3 = itr2->second.find(m_NodeId);
				if (itr3 == itr2->second.end()) continue;
				itr2->second.erase(itr3);
				if (itr2->second.empty()) m_mapSysMsgType2NodeId.erase(itr2);
			}
		}

		//加载插件
		virtual bool rpsfLoadOnePlugin(const std::string& pgname_, bool enable_, const std::map<std::string, std::string>& initParas) {
			std::pair<std::shared_ptr<ytlib::DynamicLibrary>, bool> result = GET_LIB(T_STRING_TO_TSTRING(pgname_));
			if (!result.first) {
				//没加载上
				return false;
			}
			if (result.second) {
				//已经加载过了
				return false;
			}
			typedef IPlugin* (*CreatePluginFun)(IBus*);
			CreatePluginFun CreatePlugin = (CreatePluginFun)result.first->GetSymbol(CREATE_PLUGIN_STRING);
			if (CreatePlugin) {
				IPlugin* pPlugin = CreatePlugin(this);//创建插件对象
				if (pPlugin == NULL) {
					//加载失败
					return false;
				}
				if (pgname_ != pPlugin->name) {
					//名称不对应
					return false;
				}
				pPlugin->Start(initParas);
				std::unique_lock<std::shared_mutex> lck(m_mapPgName2PgPointMutex);
				m_mapPgName2PgPoint.insert(std::pair<std::string, std::pair<IPlugin*, bool> >(pgname_, std::pair<IPlugin*, bool>(pPlugin, enable_)));
				//加载成功
			}
			return true;
		}
		//批量加载插件
		virtual void rpsfLoadPlugins(const std::set<PluginCfg>& plugins) {
			for (std::set<PluginCfg>::const_iterator itr = plugins.begin(); itr != plugins.end(); ++itr) {
				rpsfLoadOnePlugin(itr->PluginName, itr->enable, itr->InitParas);
			}
		}

		//卸载插件
		virtual bool rpsfRemoveOnePlugin(const std::string& pgname_) {
			//先失能此插件
			rpsfEnableOnePlugin(pgname_, false);
			//取消与此插件有关的一切订阅、RPC
			std::unique_lock<std::shared_mutex> lck(m_mapPgName2PgPointMutex);
			std::map<std::string, std::pair<IPlugin*, bool> >::iterator itr = m_mapPgName2PgPoint.find(pgname_);
			if (itr == m_mapPgName2PgPoint.end()) {
				//想要卸载未加载的插件
				return false;
			}

			
			itr->second.first->Stop();//停止插件
			m_mapPgName2PgPoint.erase(itr);//从表中删除
			//再卸载此插件
			if (!REMOVE_LIB(T_STRING_TO_TSTRING(pgname_))) {
				//卸载失败
				return false;
			}
			return true;
		}
		//插件不使能的意思是：插件仍然在运行，但是有数据、RPC到来时不予调用
		//使能插件
		virtual bool rpsfEnableOnePlugin(const std::string& pgname_, bool enable_ = true) {
			std::unique_lock<std::shared_mutex> lck(m_mapPgName2PgPointMutex);
			std::map<std::string, std::pair<IPlugin*, bool> >::iterator itr = m_mapPgName2PgPoint.find(pgname_);
			if (itr == m_mapPgName2PgPoint.end()) {
				//没有此插件
				return false;
			}
			itr->second.second = enable_;
			return true;
		}

		//消息分类器。从网络收到消息的回调
		void rpsfMsgClassifier(rpsfPackagePtr& pmsg) {
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
				pmsg->obj.m_srcAddr = m_NodeId;
				//如果是本地的，找出目的地，如果目的地是其他节点，则通过网络发送
				std::set<uint32_t> dst = std::move(pmsg->obj.m_pushList);//读取推送地址

				//根据数据类型，从不同的表里找目的地
				switch (pmsg->obj.m_msgType) {
				case MsgType::RPSF_SYS: {
					//找有哪些节点订阅了此系统事件
					std::shared_lock<std::shared_mutex> lck(m_mapSysMsgType2NodeIdMutex);
					std::map<SysMsgType, std::set<uint32_t> >::const_iterator itr =	m_mapSysMsgType2NodeId.find(getSysMsgTypeFromPackage(pmsg));
					if (itr != m_mapSysMsgType2NodeId.end()) dst.insert(itr->second.begin(), itr->second.end());
					break;
				}
				case MsgType::RPSF_DATA: {
					//找有哪些节点订阅了此数据
					std::shared_lock<std::shared_mutex> lck(m_mapDataNmae2NodeIdMutex);
					std::map<std::string, std::set<uint32_t> >::const_iterator itr = m_mapDataNmae2NodeId.find(getDataNameFromPackage(pmsg));
					if (itr != m_mapDataNmae2NodeId.end()) dst.insert(itr->second.begin(), itr->second.end());
					break;
				}
				case MsgType::RPSF_RPC: {
					//找有哪些节点提供此RPC服务
					std::shared_lock<std::shared_mutex> lck(m_mapService2NodeIdMutex);
					std::map<std::string, std::set<uint32_t> >::const_iterator itr = m_mapService2NodeId.find(getServiceFromPackage(pmsg));
					if (itr != m_mapService2NodeId.end()) dst.insert(itr->second.begin(), itr->second.end());
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
				//找到本地对应的插件进行并行化异步派发，以减少此步骤占用插件列表的时间
				std::shared_lock<std::shared_mutex> lck(m_mapDataName2PgNameMutex);
				std::map<std::string, std::set<std::string> >::const_iterator itr = m_mapDataName2PgName.find(data.m_dataName);
				if (itr == m_mapDataName2PgName.end())	break;
				std::vector<std::thread> onDataThreads;
				std::shared_lock<std::shared_mutex> lck2(m_mapPgName2PgPointMutex);
				for (std::set<std::string>::const_iterator itr2 = itr->second.begin(); itr2 != itr->second.end(); ++itr2) {
					std::map<std::string, std::pair<IPlugin*, bool> >::const_iterator itr3 = m_mapPgName2PgPoint.find(*itr2);
					if ((itr3 != m_mapPgName2PgPoint.end())&&(itr3->second.second)) {
						//创建线程。todo:需要测试要不要std::move?
						onDataThreads.push_back(std::thread(std::bind(&IPlugin::OnData, itr3->second.first, std::placeholders::_1), data));
					}
				}
				lck2.unlock();
				lck.unlock();
				
				//等待线程结束
				size_t len = onDataThreads.size();
				for (size_t ii = 0; ii < len; ++ii) {
					onDataThreads[ii].join();
				}

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
		virtual void rpsfSysHandler(rpsfSysMsg& m_) {
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

		//插件名称与插件指针的表
		std::shared_mutex m_mapPgName2PgPointMutex;
		std::map<std::string, std::pair<IPlugin*,bool> > m_mapPgName2PgPoint;

		//本地插件订阅的数据名称与插件名称的表
		std::shared_mutex m_mapDataName2PgNameMutex;
		std::map<std::string, std::set<std::string> > m_mapDataName2PgName;

	};


}