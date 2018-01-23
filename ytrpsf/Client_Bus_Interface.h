#pragma once
#include <ytrpsf/Plugin_Bus_Interface.h>
#include <boost/noncopyable.hpp>

//这个模块最后再弄
namespace rpsf {
	//定义客户端：使用总线的业务主动启动程序。
	//实际使用时上层只需要创建一个client实例，然后调用其接口即可
	//客户端先做简单一些，只有启动与结束本地节点的功能
	
	//！！！！！如果可以在插件中dll实现qt窗口，那么就不需要留client了，直接用bus就行了

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