#pragma once
#include <ytlib/NetTools/NetAdapterBase.h>
#include <boost/asio.hpp>

namespace ytlib {

	/*
	使用boost.asio的一个简易网络适配器
	*/
	
	struct DataPackage {
		std::shared_ptr<std::string> dataPtr;//数据
		std::vector<tstring> files;	//文件
	};

	typedef boost::asio::ip::tcp::endpoint tcped;//28个字节

	class TCPNetAdapter : public NetAdapterBase<tcped, DataPackage> {
	public:
		TCPNetAdapter(uint32_t myid_, const tcped & hostInfo_,
			std::function<void(DataPackage &)> recvcb_) :NetAdapterBase(myid_, hostInfo_, recvcb_) {

		}
		virtual ~TCPNetAdapter(){}
	protected:
		void _sendfun(const _TransData_dst& Tdata_) {
			//直接用读取锁来获取ep


		}
		//接收数据




	};



}