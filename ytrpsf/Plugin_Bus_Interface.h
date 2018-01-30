#pragma once
#include <map>
#include <set>
#include <ytlib/NetTools/SharedBuf.h>
#include <memory>

namespace rpsf {
	//本文件定义了插件和总线之间交互的接口

	//处理方式
	enum HandleType {
		RPSF_ORDER=0,//异步有序处理
		RPSF_UNORDER=1,//异步无序处理
		RPSF_SYNC=2//同步处理
	};

	//总线错误
	//BusErr总数小于256个.因为考虑跨平台时的大小端问题
	enum BusErr {
		RPSF_NO_ERROR=0,
		RPSF_FILE_NOT_EXIST,

		RPSF_SERVICE_NOT_EXIST,
		RPSF_TIMEOUT,
		RPSF_RPC_FAILED,

		RPSF_SUBSCRIBE_DATA_FAILED,
		RPSF_UNSUBSCRIBE_DATA_FAILED,

		RPSF_SUBSCRIBE_SERVICE_FAILED,
		RPSF_UNSUBSCRIBE_SERVICE_FAILED,

		RPSF_ERR_COUNT
	};

	//数据包。注意：异步调用总线的接口的，不要在调用总线接口之后去修改数据。即：可以删除、添加一条数据，甚至清空数据，但不能修改一条数据的具体内容
	class rpsfPackage {
	public:

		//此种复制方式没有复制数据，需确保buf_中的数据不会变化
		void rpsfPackage::addData_unsafe(const std::string& dataTag, const boost::shared_array<char>& buf_, uint32_t buf_size_) {
			m_mapDatas[dataTag] = std::move(ytlib::sharedBuf(buf_, buf_size_));
		}

		void rpsfPackage::addData(const std::string& dataTag, const char* buf_, uint32_t buf_size_) {
			m_mapDatas[dataTag] = std::move(ytlib::sharedBuf(buf_, buf_size_));
		}
		void rpsfPackage::addData(const std::string& dataTag, const std::string& data_) {
			m_mapDatas[dataTag] = std::move(ytlib::sharedBuf(data_));
		}

		//数据容器
		std::map<std::string, ytlib::sharedBuf > m_mapDatas;
		//文件容器
		std::map<std::string, std::string> m_mapFiles;
		bool m_bDelFiles;

		HandleType m_handleType;
	};

	class rpsfData : public rpsfPackage {
	public:
		rpsfData() {}
		rpsfData(const std::string& dataName_) :m_dataName(dataName_) {	}
		std::string m_dataName;//数据名称
	};

	class rpsfRpcArgs : public rpsfPackage {
	public:
		rpsfRpcArgs(){}
		rpsfRpcArgs(const std::string& service_) :m_service(service_){}
		std::string m_service;//服务名称
	};
	class rpsfRpcResult : public rpsfPackage {
	public:
		rpsfRpcResult(){}
		rpsfRpcResult(BusErr err):m_rpcErr(err){}

		BusErr m_rpcErr;
		std::string m_errMsg;
	};
	
	//总线基类
	class IPlugin;
	class IBus {
	public:
		IBus(){}
		virtual ~IBus(){}
		//订阅、取消订阅数据、服务
		virtual BusErr SubscribeData(const IPlugin* pPlugin_, const std::string& dataNames_) = 0;
		virtual BusErr UnsubscribeData(const IPlugin* pPlugin_, const std::string& dataName_) = 0;
		virtual std::set<std::string> GetSubscribeDataList(const IPlugin* pPlugin_) = 0;

		virtual BusErr SubscribeService(const IPlugin* pPlugin_, const std::string& service_, const std::string& remark_="") = 0;
		virtual BusErr UnsubscribeService(const IPlugin* pPlugin_, const std::string& service_) = 0;
		virtual std::map<std::string, std::string> GetSubscribeServiceList(const IPlugin* pPlugin_) = 0;

		//数据传入后将失效
		//调用总线上的服务。阻塞式的
		virtual rpsfRpcResult Invoke(rpsfRpcArgs& callArgs_, uint32_t timeout = 0) = 0;//RPC接口。timeout：等待时间，0表示永远等待
		//向总线上发布数据
		virtual BusErr PublishData(rpsfData& data_) = 0;//发布数据

		virtual const char* getBusErrMsg(BusErr err) = 0;
	};

	//定义插件：由用户开发的，实现一些接口的被动调用的独立模块
	//plugin的虚基类，直接提供给用户进行开发。用户需要继承它
	class IPlugin {
	public:
		//名称和服务需要在插件构造时确定下来并且不能改变。总线指针也需要在构造函数中被传入并且确定
		IPlugin(IBus* pBus_, const std::string& name_) :
			pBus(pBus_), name(name_) {
		}
		virtual ~IPlugin() {}
		virtual bool Start(std::map<std::string, std::string>& params_) = 0;
		virtual void Stop(void) = 0;
		virtual void OnData(const rpsfData& data_) = 0;
		virtual rpsfRpcResult Invoke(rpsfRpcArgs& callArgs_) = 0;

		//对总线接口的快捷操作
		inline BusErr SubscribeData(const std::string& dataNames_) {
			return pBus->SubscribeData(this, dataNames_);
		}

		inline BusErr UnsubscribeData(const std::string& dataNames_) {
			return pBus->UnsubscribeData(this, dataNames_);
		}
		inline std::set<std::string> GetSubscribeDataList() {
			return pBus->GetSubscribeDataList(this);
		}

		inline BusErr SubscribeService(const std::string& service_, const std::string& remark_ = "") {
			return pBus->SubscribeService(this, service_, remark_);
		}
		inline BusErr UnsubscribeService(const std::string& service_) {
			return pBus->UnsubscribeService(this, service_);
		}
		inline std::map<std::string, std::string> GetSubscribeServiceList() {
			return pBus->GetSubscribeServiceList(this);
		}


		const std::string name;
	protected:
		IBus* pBus;
	};


	//用户创建的插件需要实现此接口来返回一个IPlugin实例
#ifndef CREATE_PLUGIN_STRING
#define CREATE_PLUGIN_STRING CL_TEXT("CreatePlugin")
#endif

}