#pragma once

#include <ytlib/Common/TString.h>
#include <memory>
#include <vector>
#include <map>
#include <functional>

namespace ytlib {

	struct RecvData {
		tstring hostID;//发送方主机id
		std::vector<std::shared_ptr<std::string> > dataVec;//数据
		std::vector<tstring> files;	//接收到的所有文件的文件名
	};

	struct TransData {
		std::vector<std::shared_ptr<std::string> > dataVec;//数据
		std::vector<tstring> files;	//接收到的所有文件的文件名
		bool delFileFlag;// 发送完成,是否删除数据文件
	};

	//比较通用的网络适配器接口
	class INetAdapter {
	public:
		INetAdapter(){}
		virtual ~INetAdapter(){}

		virtual bool Send(const TransData &) = 0;//TransData中包含发送信息								 
		virtual bool Initialize(const std::map<tstring, tstring>&) = 0;//初始化
		virtual bool RegisterReceiveCallBack(std::function<void(RecvData &)>) = 0;
		virtual bool start() = 0;
		virtual bool stop() = 0;

		virtual std::map<tstring, tstring> GetMyHostInfo() = 0;
		virtual std::map<tstring, tstring> GetHostInfo(const tstring &) = 0;
		virtual bool SetHost(const tstring &, const std::map<tstring, tstring> &) = 0;
		virtual bool RemoveHost(const tstring &) = 0;
		
	};



}