#pragma once
#include <ytrpsf/Bus.h>

namespace rpsf {

	//中心节点。提供一系列管理监控接口
	class CenterNode : public Bus {
	public:
		CenterNode() :Bus() {}
		virtual ~CenterNode() {
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
			if (thisnode.NodeType != RpsfNodeType::NODETYPE_CENTER) {
				tcout << T_TEXT("error : node type mismatch! please check the cfg file and exe type.") << std::endl;
				return false;
			}


			if (!(Bus::Init(thisnode))) return false;

			//订阅系统事件
			Bus::SubscribeSysEvent(std::set<SysMsgType>{
				SysMsgType::SYS_SUB_DATAS,
				SysMsgType::SYS_SUB_SERVICES,
				SysMsgType::SYS_SUB_SYSEVENT,
				SysMsgType::SYS_HEART_BEAT_RESPONSE
			});

			//启动心跳进程
			
			m_pheartBeatThread = std::make_unique<std::thread>(std::bind(&CenterNode::heartBeatThreadFun, this));

			//加载各个插件
			rpsfLoadPlugins(thisnode.PluginSet);

			m_bRunning = true;
			return true;
		}

		virtual bool Stop() {
			if (!m_bRunning) return true;//已经停止了
			if (!Bus::Stop()) return false;
			m_pheartBeatThread->join();
			return true;
		}
	private:
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

		//心跳进程。进行监控业务，不是严格的等时进行处理
		void heartBeatThreadFun() {
			uint32_t heartBeatIndex = 0;
			while (m_bRunning) {
				std::this_thread::sleep_for(std::chrono::milliseconds(3000));//3s一次
				//心跳进程每隔一定时间广播一次心跳事件。普通节点们在收到心跳事件后向中心节点回复自身信息供中心节点监控
				//心跳进程每隔一定时间检查一个普通节点的信息
				heartBeatMsg msg;
				msg.heartBeatIndex = heartBeatIndex;


				rpsfPackagePtr p = setSysMsgToPackage(msg, SysMsgType::SYS_SUB_SYSEVENT);
				p->obj.m_handleType = HandleType::RPSF_SYNC;
				rpsfMsgHandler(p);//同步处理

				//一定次数心跳后进行一次全网信息更新
				if (heartBeatIndex % 10 == 0) {
					//全网更新一定次数后则不再进行全网更新，除非有新的表改动将次数重置为零



				}

				++heartBeatIndex;
			}
		}


		std::unique_ptr<std::thread> m_pheartBeatThread;
	};

}


