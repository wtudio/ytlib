#pragma once
#include <map>
#include <set>
#include <boost/shared_array.hpp>

namespace rpsf {
	//本文件定义了插件和总线之间交互的接口

	//处理方式
	enum HandleType {
		RPSF_ORDER,//异步有序处理
		RPSF_UNORDER,//异步无序处理
		RPSF_SYNC//同步处理
	};

	//总线错误
	enum BusErr {
		RPSF_NO_ERROR=0,
		RPSF_FILE_NOT_EXIST,

		RPSF_SERVICE_NOT_EXIST,
		RPSF_FUN_NOT_EXIST,
		RPSF_TIMEOUT,
		RPSF_RPC_FAILED
	};

	//数据包：public的是用户可以随意修改值的，private的是用户只读的
	class rpsfPackage {
	public:

		//数据容器
		std::map<std::string, std::pair<boost::shared_array<char>, uint32_t>> m_mapDatas_shared_pchar;
		std::map<std::string, std::pair<char*, uint32_t>> m_mapDatas_pchar;
		std::map<std::string, std::string*> m_mapDatas_pstring;

		//文件容器
		std::map<std::string, std::string> m_mapFiles;
		bool m_bDelFiles;

		HandleType m_handleType;
	};

	class rpsfData : public rpsfPackage {
	public:
		std::string m_dataName;//数据名称
		std::string m_sender;//数据发送者

		//获取数据发送时间
		inline std::string getDataTime() {
			return m_dataTime;
		}
	private:
		std::string m_dataTime;
	};

	class rpsfRpcArgs : public rpsfPackage {
	public:
		rpsfRpcArgs(){}
		rpsfRpcArgs(const std::string& service_, const std::string& fun_) :
			m_service(service_), m_fun(fun_) {}

		std::string m_service;//服务名称
		std::string m_fun;//函数名称
	};
	class rpsfRpcResult : public rpsfPackage {
	public:

		BusErr m_rpcErr;
		std::string m_errMsg;
	};

	//总线基类
	class IBus {
	public:
		//订阅、取消订阅数据、事件
		virtual BusErr SubscribeData(const std::string& pluginName_, const std::set<std::string>& dataNames_) = 0;
		virtual BusErr UnsubscribeData(const std::string& pluginName_, const std::set<std::string>& dataName_) = 0;
		virtual BusErr SubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_) = 0;
		virtual BusErr UnsubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_) = 0;


		virtual rpsfRpcResult Invoke(const rpsfRpcArgs& callArgs_, uint32_t timeout = 0) = 0;//RPC接口。timeout：等待时间，0表示永远等待
		virtual BusErr PublishData(const rpsfData& data_) = 0;//发布数据

		virtual const char* getBusErrMsg(BusErr err) = 0;
	};

	//plugin的虚基类，直接提供给用户进行开发。用户需要继承它
	class IPlugin {
	public:
		//名称和服务需要在插件构造时确定下来并且不能改变。总线指针也需要在构造函数中被传入并且确定
		IPlugin(IBus* pBus_, const std::string& name_, const std::set<std::string>& funs_) :
			pBus(pBus_), name(name_), funs(funs_) {
		}
		virtual ~IPlugin() {}
		virtual bool Start(std::map<std::string, std::string>& params_) = 0;
		virtual void Stop(void) = 0;
		virtual void OnData(const rpsfData& data_) = 0;
		virtual rpsfRpcResult Invoke(const rpsfRpcArgs& callArgs_) = 0;

		const std::string name;
		const std::set<std::string> funs;
	private:
		const IBus* pBus;
	};


	//用户创建的插件需要实现此接口来返回一个IPlugin实例
#ifndef CREATE_PLUGIN_STRING
#define CREATE_PLUGIN_STRING CL_TEXT("CreatePlugin")
#endif
}