#pragma once
#include <ytrpsf/RPSF_Interface/IBus.h>

namespace rpsf {
	//本文件提供给插件开发方

	//用户创建的插件需要实现此接口
#ifndef CREATE_PLUGIN_STRING
#define CREATE_PLUGIN_STRING CL_TEXT("CreatePlugin")
#endif

	//plugin的虚基类，直接提供给用户进行开发。用户需要继承它
	class IPlugin {
	public:
		//名称和服务需要在插件构造时确定下来并且不能改变
		IPlugin(IBus* pBus_, const std::string& name_, const std::set<std::string>& funs_) :
			pBus(pBus_) , name(name_), funs(funs_){	}
		virtual ~IPlugin() {}
		virtual bool Start(std::map<std::string, std::string>& params_) = 0;
		virtual void Stop(void) = 0;
		virtual void OnData(const rpsfData& data_) = 0;
		virtual void OnEvent(const rpsfEvent& event_) = 0;
		virtual rpsfResult Invoke(const rpsfCallArgs& callArgs_) = 0;

		const std::string name;
		const std::set<std::string> funs;
	private:
		IBus* pBus;
	};








}