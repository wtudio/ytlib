#pragma once
#include <boost/asio.hpp>


namespace ytlib
{
	/*
	标准数据包发送规范：
	step1：先传一个报头：（8 byte）
		head: 2 byte
		tag: 2 byte
		size: 4 byte ：默认windows，即小端
			num = byte1+byte2*256+byte3*65536+byte4*2^24
			byte1=num%256,byte2=(num/256) ...
	step2：传输size个byte的数据
	如果使用结束符的话，需要在发送完成一个包后发送一个结束head：tag = TCPEND1 + TCPEND2
	*/

	typedef boost::asio::ip::tcp::endpoint TcpEp;//28个字节
	typedef boost::asio::ip::tcp::socket TcpSocket;


	//默认vs
	static void set_buf_from_num(char* p, uint32_t n) {
#ifdef _MSC_VER
		memcpy(p, &n, 4);
#else
		p[0] = char(n % 256); n /= 256;	p[1] = char(n % 256); n /= 256;
		p[2] = char(n % 256); n /= 256;	p[3] = char(n % 256);
#endif // _MSC_VER
	}

	static uint32_t get_num_from_buf(char* p) {
#ifdef _MSC_VER
		uint32_t n;	memcpy(&n, p, 4); return n;
#else
		return (p[0] + p[1] * 256 + p[2] * 65536 + p[3] * 256 * 65536);
#endif // _MSC_VER
	}

	//检查端口是否可用。true说明可用
	static bool checkPort(uint16_t port_) {
		boost::asio::io_service io;
		TcpSocket sk(io);
		sk.open(boost::asio::ip::tcp::v4());
		boost::system::error_code err;
		sk.connect(TcpEp(boost::asio::ip::tcp::v4(), port_), err);
		if (err) return true;//连接不上，说明没有程序在监听
		sk.close();//否则说明已经被占用了
		return false;
	}

	static uint16_t getUsablePort(uint16_t start_=60000) {
		for (uint16_t ii = start_; ii < 65535; ii++) {
			if (checkPort(ii)) return ii;
		}
		//如果没有找到则返回0
		return 0;
	}
}