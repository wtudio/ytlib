#pragma once
#include <ytrpsf/Bus.h>
#include <boost/date_time/posix_time/posix_time.hpp>  
#include <chrono>

namespace rpsf {

	enum NodeState {
		NODE_STATE_NORMAL,
		NODE_STATE_DELAY,
		NODE_STATE_NO_RESPONSE,
		NODE_STATE_LOST,
		NODE_STATE_NEW_NODE
	};
	class nodeInfo {
	public:
		nodeInfo(uint32_t nodeId_,const std::string& nodeName_, std::chrono::system_clock::time_point regTime_, uint32_t curHeartBeatIndex_)
			:nodeId(nodeId_), nodeName(nodeName_), regTime(regTime_),
			curHeartBeatIndex(curHeartBeatIndex_), curPing(0), state(NodeState::NODE_STATE_NEW_NODE){

		}
		std::shared_mutex nodeInfoMutex;

		const uint32_t nodeId;
		const std::string nodeName;
		const std::chrono::system_clock::time_point regTime;//注册时间

		uint32_t curHeartBeatIndex;//心跳序号
		uint64_t curPing;//Ping，us。中心节点发送心跳信息到中心节点收到返回信息的时间

		NodeState state;//状态
		double cpuUsage;
		double memUsage;

		std::set<std::string> plugins;//加载的插件

		void updataState(uint32_t curHeartBeatIndex_) {
			//如果ping>1个心跳则判断为delay


			//如果慢3个心跳则判断为无响应

			//如果慢10个心跳则判断为掉线，将其下线
		}
	};
	typedef std::shared_ptr<nodeInfo> nodeInfoPtr;

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

			//注册自己
			std::unique_lock<std::shared_mutex> lck(m_mapNodeInfoMutex);
			m_mapNodeInfo.insert(std::pair<uint32_t, nodeInfoPtr>(m_NodeId, std::make_shared<nodeInfo>(m_NodeId, thisnode.NodeName, std::chrono::system_clock::now(), heartBeatIndex)));
			lck.unlock();


			//加载各个插件
			rpsfLoadPlugins(thisnode.PluginSet);

			m_bRunning = true;

			//启动心跳进程
			m_pheartBeatThread = std::make_unique<std::thread>(std::bind(&CenterNode::heartBeatThreadFun, this));
			//m_pheartBeatThread->detach();

			

			
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
			case SysMsgType::SYS_NEW_NODE_REG: {
				newNodeRegMsg msg;
				getSysMsgFromPackage(m_, msg);
				syncSysMsgType2NodeId(msg.SysMsg, msg.NodeId);

				newNodeRegResponseMsg msg1;
				std::shared_lock<std::shared_mutex> lck(m_mapSysMsgType2NodeIdMutex);
				msg1.mapSysMsgType2NodeId = m_mapSysMsgType2NodeId;
				lck.unlock();
				std::shared_lock<std::shared_mutex> lck1(m_mapDataNmae2NodeIdMutex);
				msg1.mapDataNmae2NodeId = m_mapDataNmae2NodeId;
				lck1.unlock();
				std::shared_lock<std::shared_mutex> lck2(m_mapService2NodeIdMutex);
				msg1.mapService2NodeId = m_mapService2NodeId;
				lck2.unlock();

				rpsfPackagePtr p1 = setSysMsgToPackage(msg1, SysMsgType::SYS_NEW_NODE_REG_RESPONSE);
				p1->obj.m_handleType = HandleType::RPSF_SYNC;
				p1->obj.m_pushList.insert(msg.NodeId);
				rpsfMsgHandler(p1);//同步处理

				std::unique_lock<std::shared_mutex> lck3(m_mapNodeInfoMutex);
				m_mapNodeInfo.insert(std::pair<uint32_t, nodeInfoPtr>(msg.NodeId,std::make_shared<nodeInfo>(msg.NodeId, msg.NodeName, std::chrono::system_clock::now(), heartBeatIndex)));

				break;
			}
			case SysMsgType::SYS_HEART_BEAT_RESPONSE: {
				heartBeatResponseMsg msg;
				getSysMsgFromPackage(m_, msg);

				std::shared_lock<std::shared_mutex> lck(m_mapNodeInfoMutex);
				std::map<uint32_t, nodeInfoPtr>::iterator itr = m_mapNodeInfo.find(msg.nodeId);
				if (itr != m_mapNodeInfo.end()) {
					std::unique_lock<std::shared_mutex> lck1(itr->second->nodeInfoMutex);
					//防止新信息在旧信息之前到
					if (itr->second->curHeartBeatIndex < msg.heartBeatIndex) {
						itr->second->curHeartBeatIndex = msg.heartBeatIndex;

						itr->second->cpuUsage = msg.cpuUsage;
						itr->second->memUsage = msg.memUsage;

						boost::posix_time::ptime tnow = boost::posix_time::microsec_clock::universal_time();
						uint64_t itnow;
						memcpy(&(itnow), &tnow, 8);
						itr->second->curPing = itnow - msg.heartBeatTime;

						//进行一次判断，修改其状态
						itr->second->updataState(heartBeatIndex);
					}
				}
				else {
					//没有找到这个node
					lck.unlock();

				}
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

			while (m_bRunning) {
				std::this_thread::sleep_for(std::chrono::milliseconds(2000));//2s一次
				//先检查一下各个节点的状态
				std::shared_lock<std::shared_mutex> lck3(m_mapNodeInfoMutex);
				for (std::map<uint32_t, nodeInfoPtr>::iterator itr = m_mapNodeInfo.begin(); itr != m_mapNodeInfo.end(); ++itr) {
					std::unique_lock<std::shared_mutex> lck1(itr->second->nodeInfoMutex);
					itr->second->updataState(heartBeatIndex);
					//调试时打印一下各个节点状态
					printf("%s-%d:%d\n", itr->second->nodeName.c_str(), itr->second->nodeId, itr->second->curHeartBeatIndex);

				}
				lck3.unlock();
				//心跳进程每隔一定时间广播一次心跳事件。普通节点们在收到心跳事件后向中心节点回复自身信息供中心节点监控
				//心跳进程每隔一定时间检查一个普通节点的信息
				heartBeatMsg msg;
				msg.heartBeatIndex = heartBeatIndex;
				boost::posix_time::ptime tnow = boost::posix_time::microsec_clock::universal_time();
				memcpy(&(msg.heartBeatTime), &tnow, 8);

				rpsfPackagePtr p = setSysMsgToPackage(msg, SysMsgType::SYS_HEART_BEAT);
				p->obj.m_handleType = HandleType::RPSF_SYNC;
				///rpsfMsgHandler(p);//同步处理

				//一定次数心跳后进行一次全网信息更新
				if ((heartBeatIndex % 2 == 0) && (infoChangeCount<3)) {
					++infoChangeCount;
					//全网更新一定次数后则不再进行全网更新，除非有新的表改动将次数重置为零
					allInfoMsg msg1;
					std::shared_lock<std::shared_mutex> lck(m_mapSysMsgType2NodeIdMutex);
					msg1.mapSysMsgType2NodeId = m_mapSysMsgType2NodeId;
					lck.unlock();
					std::shared_lock<std::shared_mutex> lck1(m_mapDataNmae2NodeIdMutex);
					msg1.mapDataNmae2NodeId = m_mapDataNmae2NodeId;
					lck1.unlock();
					std::shared_lock<std::shared_mutex> lck2(m_mapService2NodeIdMutex);
					msg1.mapService2NodeId = m_mapService2NodeId;
					lck2.unlock();

					rpsfPackagePtr p1 = setSysMsgToPackage(msg1, SysMsgType::SYS_ALL_INFO);
					p1->obj.m_handleType = HandleType::RPSF_UNORDER;
					///rpsfMsgClassifier(p1);//无序通道处理
				}

				++heartBeatIndex;
				
			}
		}


		std::unique_ptr<std::thread> m_pheartBeatThread;
		std::atomic_uint32_t infoChangeCount = 0;
		std::atomic_uint32_t heartBeatIndex = 0;

		std::shared_mutex m_mapNodeInfoMutex;//只在增减m_mapNodeInfo的数量时使用
		std::map<uint32_t, nodeInfoPtr> m_mapNodeInfo;

	};

}


