#pragma once
#include <boost/asio.hpp>

namespace ytlib
{
	/*
	标准数据包发送规范：
	step1：先传一个报头：（8 byte）
		head: 2 byte
		tag: 2 byte
		size: 4 byte ：默认大端传输
	step2：传输size个byte的数据
	如果使用结束符的话，需要在发送完成一个包后发送一个结束head：tag = TCPEND1 + TCPEND2
	*/

	typedef boost::asio::ip::tcp::endpoint TcpEp;//28个字节
	typedef boost::asio::ip::tcp::socket TcpSocket;

	//大小端转换，将ps中的数据转换到pd中
	static void transEndian(char* pd, const char* ps, size_t len) {
#ifdef BIG_ENDIAN
		memcpy(pd, ps, len);
#else
		ps += len;
		while (len--) (*(pd++)) = (*(--ps));
#endif 
	}

	static void set_buf_from_num(char* p, uint32_t n) {
#ifdef BIG_ENDIAN
		memcpy(p, &n, 4);
#else
		p[0] = ((char*)& n)[3];
		p[1] = ((char*)& n)[2];
		p[2] = ((char*)& n)[1];
		p[3] = ((char*)& n)[0];
#endif 
	}

	static uint32_t get_num_from_buf(char* p) {
		uint32_t n;
#ifdef BIG_ENDIAN
		memcpy(&n, p, 4); 
#else
		((char*)& n)[3] = p[0];
		((char*)& n)[2] = p[1];
		((char*)& n)[1] = p[2];
		((char*)& n)[0] = p[3];
#endif 
		return n;
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