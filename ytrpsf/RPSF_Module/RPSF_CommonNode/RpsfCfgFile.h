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

	class RpsfNode {
	public:
		enum RpsfNodeType {
			NODETYPE_COMMON,
			NODETYPE_CENTER
		};

		RpsfNode() :UseNetLog(false), NodeType(NODETYPE_COMMON){

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


	};
	
	class RpsfCfgFile : public ytlib::FileBase<RpsfNode> {
	public:
		RpsfCfgFile() : ytlib::FileBase() {

		}
		~RpsfCfgFile(){}
	protected:


	};


}


