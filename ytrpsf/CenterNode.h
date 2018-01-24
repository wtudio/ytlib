#pragma once
#include <ytrpsf/Bus.h>

namespace rpsf {

	//中心节点。提供一系列管理监控接口
	class CenterNode {
	private:
		bool m_bInit;
		bool m_bRunning;

		Bus m_bus;

	public:
		CenterNode() :m_bInit(false), m_bRunning(false) {}
		virtual ~CenterNode() { stop(); }

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
			if (thisnode.NodeType != RpsfNodeType::NODETYPE_CENTER) {
				tcout << T_TEXT("error : node type mismatch! please check the cfg file and exe type.") << std::endl;
				return false;
			}




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


