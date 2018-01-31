#pragma once

#include <ytlib/Common/Util.h>
#include <ytlib/Common/TString.h>
#include <sstream>
#include <fstream>
#include <boost/smart_ptr.hpp>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/shared_ptr_132.hpp>
#include <boost/serialization/shared_ptr_helper.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/smart_cast.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/queue.hpp>
#include <boost/serialization/stack.hpp>
#include <boost/serialization/deque.hpp>

#include <string>
#include <memory>

namespace ytlib
{
	enum SerializeType
	{
		TextType,
		BinaryType
	};
//侵入式序列化类头
#define T_CLASS_SERIALIZE(Members) \
	friend class boost::serialization::access; \
	template<class Archive> \
	void serialize(Archive & ar, const uint32_t version) \
	{ ar Members; }
	

	//提供的函数仅适用于序列化规模较小，需要直接转化为string或存储成文件，不太考虑性能的场合
	template<class T>
	bool Serialize(const T& obj, std::string &data, SerializeType Type = TextType) {
		try {
			std::ostringstream oss(std::ios_base::binary);
			if (Type== TextType) {
				boost::archive::text_oarchive oar(oss);
				oar << obj;
			}
			else  {
				boost::archive::binary_oarchive oar(oss);
				oar << obj;
			}
			data = std::move(oss.str());
			return true;
		}
		catch (const std::exception& e) {
			std::cout << "Serialize failed:" << e.what() << std::endl;
			return false;
		}
	}

	template<class T>
	bool Deserialize(T& obj, const std::string &data, SerializeType Type = TextType) {
		try {
			std::istringstream iss(std::move(data), std::ios_base::binary);
			if (Type == TextType) {
				boost::archive::text_iarchive iar(iss);
				iar >> obj;
			}
			else {
				boost::archive::binary_iarchive iar(iss);
				iar >> obj;
			}
			return true;
		}
		catch (const std::exception& e) {
			std::cout << "Deserialize failed:" << e.what() << std::endl;
			return false;
		}
	}

	template<class T>
	bool Serialize_f(const T & obj, const tstring & filepath, SerializeType Type = TextType) {
		try {
			std::ofstream oss(filepath.c_str(), std::ios::trunc | std::ios::binary);
			if (!oss) return false;
			if (Type == TextType) {
				boost::archive::text_oarchive oar(oss);
				oar << obj;
			}
			else {
				boost::archive::binary_oarchive oar(oss);
				oar << obj;
			}
			oss.close();
			return true;
		}
		catch (const std::exception& e) {
			std::cout << "Serialize failed:" << e.what() << std::endl;
			return false;
		}
	}

	template<class T>
	bool Deserialize_f(T& obj, const tstring & filepath, SerializeType Type = TextType) {
		try {
			std::ifstream iss(filepath.c_str(), std::ios::binary);
			if (!iss) return false;
			if (Type == TextType) {
				boost::archive::text_iarchive iar(iss);
				iar >> obj;
			}
			else {
				boost::archive::binary_iarchive iar(iss);
				iar >> obj;
			}
			iss.close();
			return true;
		}
		catch (const std::exception& e) {
			std::cout << "Deserialize failed:" << e.what() << std::endl;
			return false;
		}
	}

	//高性能序列化宏定义，线程不安全，脱离当前作用域p和len就会失效
	//先SERIALIZE_INIT，在SERIALIZE_INIT所在作用域内SERIALIZE和其产生的p和len有效
	//后一次SERIALIZE时将清空前一次SERIALIZE的结果

	//高性能反序列化宏定义
	//先DESERIALIZE_INIT，在DESERIALIZE_INIT所在作用域内DESERIALIZE有效
	//安全性存疑，可能会发生析构时内存错误（待测试）
	//默认使用二进制方式序列化
	class mystreambuf :public std::basic_stringbuf<char, std::char_traits<char>, std::allocator<char> >
	{
		friend class myostringstream;
		friend class myistringstream;
	public:
		~mystreambuf() {}
	};

	class myostringstream :public std::ostringstream
	{
	public:
		myostringstream(ios_base::openmode _Mode = ios_base::out) :std::ostringstream(_Mode) {}
		~myostringstream() {};
		void getPoint(char* &p, size_t &len) {
			_mybuf.str("");
			rdbuf()->swap(_mybuf);
			p = _mybuf.pbase();
			len = _mybuf.pptr() - p;
			rdbuf()->swap(_mybuf);
		}
	private:
		mystreambuf _mybuf;
	};

	class myistringstream :public std::istringstream
	{
	public:
		myistringstream(ios_base::openmode _Mode = ios_base::out) :std::istringstream(_Mode) {}
		~myistringstream() {};
		void setPoint(char* p, size_t len) {
			rdbuf()->swap(_mybuf);
			_mybuf.str("");
			_mybuf.setg(p, p, p + len);
			rdbuf()->swap(_mybuf);
		}
	private:
		mystreambuf _mybuf;
	};

	//用户也可以参考这些宏来定制自己的序列化反序列化操作
#define SERIALIZE_INIT \
	ytlib::myostringstream myostringstream_tmp(std::ios_base::binary); \
	std::shared_ptr<boost::archive::binary_oarchive> oar_tmp;

#define SERIALIZE(obj,p,len) \
	myostringstream_tmp.str(""); \
	oar_tmp=std::make_shared<boost::archive::binary_oarchive>(myostringstream_tmp); \
	*oar_tmp << obj; \
	myostringstream_tmp.getPoint(p, len);

#define DESERIALIZE_INIT \
	ytlib::myistringstream myistringstream_tmp(std::ios_base::binary); \
	std::shared_ptr<boost::archive::binary_iarchive> iar_tmp;

#define DESERIALIZE(obj,p,len) \
	myistringstream_tmp.setPoint(p,len); \
	iar_tmp==std::make_shared<boost::archive::binary_iarchive>(myistringstream_tmp); \
	*iar_tmp >> obj; 


}

