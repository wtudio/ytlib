#pragma once
#include <boost/shared_array.hpp>
#include <string>

namespace ytlib
{
	//通用buff
	class sharedBuf {
	public:
		boost::shared_array<char> buf;
		uint32_t buf_size;

		sharedBuf():buf_size(0){}
		sharedBuf(const boost::shared_array<char>& buf_, uint32_t buf_size_):
			buf(buf_), buf_size(buf_size_){
		}
		sharedBuf(const char* buf_, uint32_t buf_size_) :	buf_size(buf_size_) {
			buf = boost::shared_array<char>(new char[buf_size]);
			memcpy(buf.get(), buf_, buf_size);
		}
		sharedBuf(uint32_t buf_size_) : buf_size(buf_size_) {
			buf = boost::shared_array<char>(new char[buf_size]);
		}
		sharedBuf(const std::string& buf_):buf_size(buf_.size()){
			memcpy(buf.get(), buf_.c_str(), buf_size);
		}
		sharedBuf getCopy() {
			return sharedBuf(buf.get(), buf_size);
		}
	};

}