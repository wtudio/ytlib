#pragma once
#include <ytrpsf/RPSF_Interface/IPlugin.h>
#include <iostream>
//提供给RPC调用方的

class para2 {
public:
	para2(){}
	boost::shared_array<char> buf_;
	uint32_t buf_size_;
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
		rpsf::rpsfCallArgs callargs(service, "testRPC");
		callargs.addData("buf", p_.buf_, p_.buf_size_);
		callargs.addFile("f", p_.file1);
		rpsf::rpsfResult re = m_pBus->Invoke(callargs);
		if (!re) {
			std::cout << re.getErrorInfo() << std::endl;
			return false;
		}
		boost::shared_array<char> buf_; 
		uint32_t buf_size_;
		re.getData("msg", buf_, buf_size_);
		r_.remsg = std::move(std::string(buf_.get(),buf_size_));
		re.getFile("f", r_.refile);
		return true;
	}


	const std::string service;
private:
	rpsf::IBus* m_pBus;
};
