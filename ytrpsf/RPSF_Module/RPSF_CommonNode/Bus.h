#pragma once
#include <ytrpsf/RPSF_Interface/IPlugin.h>
#include <ytrpsf/RPSF_Module/RPSF_CommonNode/Msg.h>

namespace rpsf {

	class Bus {
	public:
		Bus() {

		}





	};

#define BUS(p) static_cast<Bus*>(p)
	IBus::IBus(void* p_) :p(p_){}

	bool IBus::SubscribeData(const std::string& pluginName_, const std::set<std::string>& dataNames_) {

	}
	bool IBus::UnsubscribeData(const std::string& pluginName_, const std::set<std::string>& dataName_) {

	}
	bool IBus::SubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_) {

	}
	bool IBus::UnsubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_) {

	}

	rpsfResult IBus::Invoke(const rpsfCallArgs& callArgs_) {

	}
	bool IBus::PublishEvent(const rpsfEvent& event_) {

	}
	bool IBus::PublishData(const rpsfData& data_) {

	}

	std::string IBus::GetSendFilePath() {

	}
	std::string IBus::GetRecvFilePath() {

	}
}