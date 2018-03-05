#pragma once
#include <ytlib/FileManager/FileBase.h>
#include <ytlib/SupportTools/XMLTools.h>

namespace rpsf {


	class PluginCfg {
	public:
		PluginCfg(const std::string& name_) :PluginName(name_), enable(false) {

		}
		bool operator <(const PluginCfg &p) const {
			return (PluginName < p.PluginName);
		}
		bool operator >(const PluginCfg &p) const {
			return (PluginName > p.PluginName);
		}
		bool operator ==(const PluginCfg &p) const {
			return (PluginName == p.PluginName);
		}
		bool operator !=(const PluginCfg &p) const {
			return (PluginName != p.PluginName);
		}

		std::string PluginName;
		bool enable;
		std::map<std::string, std::string> InitParas;
	};

	enum RpsfNodeType {
		NODETYPE_COMMON,
		NODETYPE_CENTER
	};

	class RpsfNode {
	public:

		RpsfNode() :UseNetLog(false), NodeType(RpsfNodeType::NODETYPE_COMMON){

		}
		
		//自身系统信息
		uint32_t NodeId;
		RpsfNodeType NodeType;
		std::string NodeName;
		uint16_t NodePort;

		//中心节点信息
		uint32_t CenterNodeId;
		std::string CenterNodeIp;
		uint16_t CenterNodePort;

		//日志配置
		bool UseNetLog;
		std::string LogServerIp;
		uint16_t LogServerPort;

		//插件配置
		std::set<PluginCfg> PluginSet;

		//可选的设置，包括：recvpath、sendpath
		std::map<ytlib::tstring, ytlib::tstring> NodeSettings;
	};
	
	class RpsfCfgFile : public ytlib::FileBase<RpsfNode> {
	public:
		RpsfCfgFile() : ytlib::FileBase<RpsfNode>() {

		}
		~RpsfCfgFile(){}
	protected:
		bool CheckFileName(const ytlib::tstring& filename) const {
			ytlib::tstring Suffix(T_TEXT("xcfg"));
			return (filename.substr(filename.length() - Suffix.length(), Suffix.length()) == Suffix);
		}
		bool GetFileObj() {
			if (!CreateFileObj()) return false;
			ytlib::tpath path = ytlib::tGetAbsolutePath(m_filepath);
			ytlib::tptree ptRoot;
			if (!ytlib::realXml(path.string<ytlib::tstring>(), ptRoot)) return false;
			try {
				ytlib::tptree ptsys = ptRoot.get_child(T_TEXT("system"));
				m_fileobj->NodeId= ptsys.get<uint32_t>(T_TEXT("<xmlattr>.id"));
				m_fileobj->NodeName = T_TSTRING_TO_STRING(ptsys.get<ytlib::tstring>(T_TEXT("<xmlattr>.name")));
				m_fileobj->NodePort = ptsys.get<uint16_t>(T_TEXT("<xmlattr>.port"));
				if (ptsys.get<ytlib::tstring>(T_TEXT("<xmlattr>.type")) == T_TEXT("CenterNode")) {
					m_fileobj->NodeType = RpsfNodeType::NODETYPE_CENTER;
				}
				else {
					m_fileobj->NodeType = RpsfNodeType::NODETYPE_COMMON;
					ytlib::tptree ptctnode = ptsys.get_child(T_TEXT("CenterNode"));
					m_fileobj->CenterNodeId = ptctnode.get<uint32_t>(T_TEXT("<xmlattr>.id"));
					m_fileobj->CenterNodeIp = T_TSTRING_TO_STRING(ptctnode.get<ytlib::tstring>(T_TEXT("<xmlattr>.Ip")));
					m_fileobj->CenterNodePort = ptctnode.get<uint16_t>(T_TEXT("<xmlattr>.port"));
				}
				boost::optional<ytlib::tptree&> ptnetlog = ptsys.get_child_optional(T_TEXT("Netlog"));
				if (ptnetlog) {
					m_fileobj->UseNetLog = true;
					m_fileobj->LogServerIp= T_TSTRING_TO_STRING(ptnetlog->get<ytlib::tstring>(T_TEXT("<xmlattr>.Ip")));
					m_fileobj->LogServerPort= ptnetlog->get<uint16_t>(T_TEXT("<xmlattr>.port"));
				}
				boost::optional<ytlib::tptree&> ptpgs = ptsys.get_child_optional(T_TEXT("plugins"));
				if (ptpgs) {
					for (ytlib::tptree::iterator itrpg = ptpgs->begin(); itrpg != ptpgs->end(); ++itrpg) {
						ytlib::tptree &Itempt = itrpg->second;
						std::string pgname = T_TSTRING_TO_STRING(Itempt.get<ytlib::tstring>(T_TEXT("<xmlattr>.libname")));
						if (pgname.empty()) {
							tcout << T_TEXT("Invalid Item Name!") << std::endl;
							return false;
						}
						PluginCfg pg(pgname);
						pg.enable = Itempt.get<bool>(T_TEXT("<xmlattr>.enable"));
						if (!ytlib::readSettings(Itempt, pg.InitParas)) return false;
						m_fileobj->PluginSet.insert(std::move(pg));
					}
				}
				if (!ytlib::readSettings(ptsys, m_fileobj->NodeSettings)) return false;
			}
			catch (const std::exception& e) {
				std::cout << "load rpsf cfg file failed : " << e.what() << std::endl;
				return false;
			}
			return true;
		}

		bool SaveFileObj() {
			//暂时用不到
			return true;
		}
	};


}


