#pragma once
#include <ytrpsf/Plugin_Bus_Interface.h>
#include <boost/noncopyable.hpp>

//这个模块最后再弄
namespace rpsf {
	//定义客户端：使用总线的业务主动启动程序。
	//其自身是一个普通节点，能够控制这个节点的启动停止，并能够将自定义的插件动态添加到自身节点上来和总线上其他节点的插件互动
	
	//本文件定义了总线提供给客户端的接口
	class Client : boost::noncopyable {
	public:
		//总线控制接口。初始化即start。析构即停止
		Client();
		virtual ~Client();
		bool Init(const std::string& cfgpath);

		bool AddPlugin(IPlugin* p_plugin);//业务接口。用户将自己实现的插件添加到总线上

		IBus* get_pIBus();
	private:
		void* p;
	};

}