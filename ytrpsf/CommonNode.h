#pragma once
#include <ytrpsf/RpsfCfgFile.h>
#include <ytrpsf/Bus.h>
#include <ytlib/SupportTools/SysInfoTools.h>
#include <boost/date_time/posix_time/posix_time.hpp>  

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
			
			//订阅系统事件
			std::set<SysMsgType> sysevents{
				SysMsgType::SYS_NEW_NODE_ONLINE,
				SysMsgType::SYS_ALL_INFO,
				SysMsgType::SYS_SUB_DATAS,
				SysMsgType::SYS_SUB_SERVICES,
				SysMsgType::SYS_SUB_SYSEVENT,
				SysMsgType::SYS_HEART_BEAT };
			Bus::SubscribeSysEvent(sysevents);
			//注册
			
			newNodeRegMsg msg{ m_NodeId ,sysevents ,thisnode.NodeName,thisnode.NodeIp,thisnode.NodePort };
			rpsfPackagePtr p = setSysMsgToPackage(msg, SysMsgType::SYS_NEW_NODE_REG);
			p->obj.m_handleType = HandleType::RPSF_SYNC;
			p->obj.m_pushList.insert(thisnode.CenterNodeId);
			regFlag.reset();
			rpsfMsgHandler(p);//同步处理

			//等待接收到回复信息。如果一定时间没有注册成功则提示注册失败
			if (!regFlag.wait_for(5000)) {
				//等了5s还未收到回复信息
				tcout << T_TEXT("Registration failed: timeout!") << std::endl;
				return false;
			}
			//注册成功，订阅系统监控事件
			SubscribeSysEvent(std::set<SysMsgType>{SysMsgType::SYS_HEART_BEAT});

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
			rpsfMsgHandler(p);//同步处理
		}
		virtual void UnsubscribeSysEvent(const std::set<SysMsgType>& sysEvents_) {
			Bus::UnsubscribeSysEvent(sysEvents_);
			subscribeSysEventMsg msg{ m_NodeId ,sysEvents_ ,false };
			rpsfPackagePtr p = setSysMsgToPackage(msg, SysMsgType::SYS_SUB_SYSEVENT);
			p->obj.m_handleType = HandleType::RPSF_SYNC;
			rpsfMsgHandler(p);//同步处理
		}
		virtual void rpsfSysHandler(rpsfSysMsg& m_) {
			//跟新表时要注意，只能跟新其他节点的信息。自己本地的信息如果跟发过来的不一样需要向上纠正
			switch (m_.m_sysMsgType) {
			case SysMsgType::SYS_HEART_BEAT: {
				//收到心跳事件
				heartBeatMsg msg;
				getSysMsgFromPackage(m_, msg);
				heartBeatResponseMsg msg2{ m_NodeId,msg.heartBeatIndex ,msg.heartBeatTime,ytlib::GetCpuUsage(),ytlib::GetMemUsage() };

				rpsfPackagePtr p = setSysMsgToPackage(msg2, SysMsgType::SYS_HEART_BEAT_RESPONSE);
				p->obj.m_handleType = HandleType::RPSF_SYNC;
				rpsfMsgHandler(p);//同步处理

				break;
			}
			case SysMsgType::SYS_ALL_INFO: {
				//收到跟新全部信息事件
				allInfoMsg msg;

				getSysMsgFromPackage(m_, msg);

				//更新系统事件与节点id的表
				std::set<SysMsgType> delset, addset;
				syncSysMsgType2NodeId(msg.mapSysMsgType2NodeId, delset, addset);
				if (!delset.empty()) UnsubscribeSysEvent(delset);
				if (!addset.empty()) SubscribeSysEvent(addset);




				break;
			}
			case SysMsgType::SYS_NEW_NODE_REG_RESPONSE: {
				//收到新节点注册响应事件
				newNodeRegResponseMsg msg;
				getSysMsgFromPackage(m_, msg);

				//更新系统事件与节点id的表
				std::set<SysMsgType> delset, addset;
				syncSysMsgType2NodeId(msg.mapSysMsgType2NodeId, delset, addset);
				if (!delset.empty()) UnsubscribeSysEvent(delset);
				if (!addset.empty()) SubscribeSysEvent(addset);




				regFlag.notify();
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


		ytlib::LightSignal regFlag;

	};

}


