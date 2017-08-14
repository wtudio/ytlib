#pragma once
#include <ytrpsf/RPSF_Interface/Plugin_Bus_Interface.h>
#include <iostream>
//提供给RPC调用方的

class para2 {
public:
	para2(){}
	boost::shared_array<char> buf_;
	uint32_t buf_size_;
	std::string data1;
	std::string file1;
};

class result1 {
public:
	std::string remsg;
	std::string refile;
};

class DemoPlugin1_Client {

public:
	DemoPlugin1_Client(rpsf::IBus* pBus):m_pBus(pBus), service("DemoPlugin1"){}


	//需要和处理处的程序一致
	bool testRPC(const para2& p_, result1& r_) {
		rpsf::rpsfRpcArgs callargs(service, "testRPC");
		callargs.addData("buf", p_.buf_, p_.buf_size_);
		callargs.m_mapFiles["f"] = p_.file1;
		rpsf::rpsfRpcResult re = m_pBus->Invoke(callargs);
		if (re.m_rpcErr) {
			std::cout << m_pBus->getBusErrMsg(re.m_rpcErr) << std::endl;
			if (!re.m_errMsg.empty()) std::cout << re.m_errMsg << std::endl;
			return false;
		}
		std::map<std::string, std::pair<boost::shared_array<char>, uint32_t>>::iterator itr = re.m_mapDatas.find("msg");
		if (itr == re.m_mapDatas.end()) return false;
		r_.remsg= std::move(std::string(itr->second.first.get(), itr->second.second));
		std::map<std::string, std::string>::iterator itrf = re.m_mapFiles.find("f");
		if (itrf == re.m_mapFiles.end()) return false;
		r_.refile = itrf->second;
		return true;
	}


	const std::string service;
private:
	rpsf::IBus* m_pBus;
};
