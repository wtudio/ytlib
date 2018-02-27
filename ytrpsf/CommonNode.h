#pragma once
#include <ytrpsf/RpsfCfgFile.h>
#include <ytrpsf/Bus.h>

namespace rpsf {
	//普通节点
	class CommonNode : public Bus{
	public:
		CommonNode() :Bus() {}
		virtual ~CommonNode() {
			Stop();
		}

		virtual bool Init(const std::string& cfgpath) {

			RpsfCfgFile cfgfile;
			try {
				cfgfile.OpenFile(T_STRING_TO_TSTRING(cfgpath));
			}
			catch (const ytlib::Exception& e) {
				std::cout << e.what() << std::endl;
				return false;
			}
			RpsfNode& thisnode = *(cfgfile.m_fileobj.get());
			if (thisnode.NodeType != RpsfNodeType::NODETYPE_COMMON) {
				tcout << T_TEXT("error : node type mismatch! please check the cfg file and exe type.") << std::endl;
				return false;
			}

			if (!(Bus::Init(thisnode))) return false;

			//载入中心节点信息
			m_netAdapter->SetHost(thisnode.CenterNodeId, thisnode.CenterNodeIp, thisnode.CenterNodePort);
			std::unique_lock<std::shared_mutex> lck(m_mapSysMsgType2NodeIdMutex);
			m_mapSysMsgType2NodeId[SysMsgType::SYS_NEW_NODE_REG].insert(thisnode.CenterNodeId);
			lck.unlock();
			//订阅系统事件
			Bus::SubscribeSysEvent(std::set<SysMsgType>{
				SysMsgType::SYS_NEW_NODE_ONLINE,
				SysMsgType::SYS_ALL_INFO,
				SysMsgType::SYS_SUB_DATAS,
				SysMsgType::SYS_SUB_SERVICES,
				SysMsgType::SYS_SUB_SYSEVENT,
				SysMsgType::SYS_HEART_BEAT
			});
			//注册


			//等待接收到回复信息


			//注册成功，订阅系统监控事件

			//加载各个插件
			rpsfLoadPlugins(thisnode.PluginSet);

			m_bRunning = true;
			return true;

		}


	private:
		virtual void SubscribeSysEvent(const std::set<SysMsgType>& sysEvents_) {
			Bus::SubscribeSysEvent(sysEvents_);
			subscribeSysEventMsg msg{ m_NodeId ,sysEvents_ ,true };
			rpsfPackagePtr p = setSysMsgToPackage(msg, SysMsgType::SYS_SUB_SYSEVENT);
			p->obj.m_handleType = HandleType::RPSF_SYNC;
			rpsfMsgHandlerLocal(p);//同步处理
		}
		virtual void UnsubscribeSysEvent(const std::set<SysMsgType>& sysEvents_) {
			Bus::UnsubscribeSysEvent(sysEvents_);
			subscribeSysEventMsg msg{ m_NodeId ,sysEvents_ ,false };
			rpsfPackagePtr p = setSysMsgToPackage(msg, SysMsgType::SYS_SUB_SYSEVENT);
			p->obj.m_handleType = HandleType::RPSF_SYNC;
			rpsfMsgHandlerLocal(p);//同步处理
		}
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
				Bus::rpsfSysHandler(m_);
				break;
			}
		}

	};

}


