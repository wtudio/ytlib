#pragma once
#include <ytrpsf/RpsfCfgFile.h>
#include <ytrpsf/Bus.h>

namespace rpsf {

	//普通节点，使用时：初始化-》开始-》结束，单向，线程不安全
	class CommonNode {
	private:
		bool m_bInit;
		bool m_bRunning;

		Bus m_bus;

	public:
		CommonNode() :m_bInit(false), m_bRunning(false) {}
		virtual ~CommonNode() { stop(); }

		virtual bool init(const std::string& cfgpath) {

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

			//设置文件收发路径
			ytlib::tstring nodename(T_STRING_TO_TSTRING(thisnode.NodeName));
			ytlib::tstring recvpath(nodename + T_TEXT("/recv"));
			ytlib::tstring sendpath(nodename + T_TEXT("/send"));
			std::map<ytlib::tstring, ytlib::tstring>::iterator itr = thisnode.NodeSettings.find(T_TEXT("recvpath"));
			if (itr != thisnode.NodeSettings.end()) recvpath = itr->second;
			itr = thisnode.NodeSettings.find(T_TEXT("sendpath"));
			if (itr != thisnode.NodeSettings.end()) sendpath = itr->second;

			if (!(m_bus.Init(thisnode.NodeId, thisnode.NodePort, recvpath, sendpath))) return false;


			m_bInit = true;

			return m_bInit;
		}

		virtual bool start() {
			if (!m_bInit)return false;



			m_bRunning = true;
			return m_bRunning;
		}

		virtual bool stop() {
			if (!m_bRunning) return true;//已经停止了



			m_bRunning = false;

			return !m_bRunning;
		}

	};

}


