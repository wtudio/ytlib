#pragma once
#include <ytrpsf/RPSF_Interface/IPlugin.h>

namespace rpsf {
	class Bus {
	public:






	};




#define PBUS(p) static_cast<Bus*>(p)

	bool IBus::SubscribeData(IPlugin* plugin_, const std::string& dataName_) {
		
	}
	bool IBus::UnsubscribeData(IPlugin* plugin_, const std::string& dataName_) {

	}
	bool IBus::SubscribeEvent(IPlugin* plugin_, const std::string& eventName_) {

	}
	bool IBus::UnsubscribeEvent(IPlugin* plugin_, const std::string& eventName_) {

	}
}