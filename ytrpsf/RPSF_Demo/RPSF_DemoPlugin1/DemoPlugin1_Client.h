#pragma once
#include <ytrpsf/RPSF_Interface/IPlugin.h>
#include <ytlib/SupportTools/Serialize.h>//此处使用了ytlib的序列化工具。用户可以使用自己的序列化工具
//提供给RPC调用方的

class para1 {
	T_CLASS_SERIALIZE(&a&b)
public:
	uint32_t a;
	std::string b;
};
class para2 {
public:
	para1 p1;
	const boost::shared_array<char>& buf_;
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
		rpsf::rpsfResult re;
		rpsf::rpsfCallArgs callargs(service, "testRPC");
		std::string s;
		if (!ytlib::Serialize(p_.p1, s)) return false;
		callargs.addCallArgsData("p1", s);
		callargs.addCallArgsData("buf", p_.buf_, p_.buf_size_);
		callargs.addCallArgsFile("f", p_.file1);
		m_pBus->Invoke(callargs, re);
		if (!re.is_success()) {
			std::cout << re.getErrorInfo() << std::endl;
			return false;
		}
		boost::shared_array<char> buf_; 
		uint32_t buf_size_;
		re.getResultData("msg", buf_, buf_size_);
		r_.remsg = std::move(std::string(buf_.get(),buf_size_));
		re.getResultFile("f", r_.refile);
		return true;
	}


	const std::string service;
private:
	rpsf::IBus* m_pBus;
};
