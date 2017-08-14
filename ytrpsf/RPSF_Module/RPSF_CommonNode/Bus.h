#pragma once
#include <ytrpsf/RPSF_Interface/Plugin_Bus_Interface.h>
#include <ytrpsf/RPSF_Interface/Client_Bus_Interface.h>

#include <ytrpsf/RPSF_Module/RPSF_CommonNode/Msg.h>
#include <ytlib/LogService/Logger.h>
#include <ytlib/SupportTools/ChannelBase.h>


namespace rpsf {

	static const char* busErrMsg[RPSF_ERR_COUNT] =
	{
		"successful.",
		"file not exist.",
		"service not exist."
		"function not exist."
		"RPC timeout.",
		"RPC failed."
	};

	class Bus : public IBus {
	public:
		//接口：bus-client
		Bus() {

		}
		~Bus() {

		}
		bool Init(const std::string& cfgpath) {

		}

		bool AddPlugin(IPlugin* p_plugin) {

		}
		//接口：bus-plugin
		BusErr SubscribeData(const std::string& pluginName_, const std::set<std::string>& dataNames_) {

		}
		BusErr UnsubscribeData(const std::string& pluginName_, const std::set<std::string>& dataName_) {

		}
		BusErr SubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_) {

		}
		BusErr UnsubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_) {

		}

		rpsfRpcResult Invoke(const rpsfRpcArgs& callArgs_, uint32_t timeout ) {

		}
		BusErr PublishData(const rpsfData& data_) {

		}

		const char* getBusErrMsg(BusErr err) {

		}
		
	private:
		//所有成员都只能用智能指针了
		bool m_bInit;


		std::shared_ptr<ytlib::TcpNetAdapter<rpsfMsg> >  m_netAdapter;//网络适配器
		std::shared_ptr<ytlib::ChannelBase<rpsfPackagePtr> > m_orderChannel;//有序通道
		std::shared_ptr<ytlib::ChannelBase<rpsfPackagePtr> > m_unorderChannel;//无序通道

	};

#define PBUS(p) static_cast<Bus*>(p)

	Client::Client() {
		p = new Bus();
	}
	Client::~Client() {
		delete p;
	}
	bool Client::Init(const std::string& cfgpath) {
		if(p) return PBUS(p)->Init(cfgpath);
		return false;
	}

	bool Client::AddPlugin(IPlugin* p_plugin) {
		if (p) return PBUS(p)->AddPlugin(p_plugin);
		return false;
	}

	IBus* Client::get_pIBus() {
		if (p) return PBUS(p);
		return NULL;
	}
}