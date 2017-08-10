#pragma once
#include <ytlib/NetTools/TcpNetUtil.h>
#include <ytlib/NetTools/TcpNetAdapter.h>

namespace rpsf {

	//本文件中定义一些数据包格式。其中需要给外界提供的数据包需要另外做接口

	//用于传输大量数据
	class rpsfData {
		//只需要序列化前两个
		T_CLASS_SERIALIZE(&m_name&m_sender)
	public:
		rpsfData() {}
		rpsfData(const std::string& name_, const std::string& sender_) :
			m_name(name_), m_sender(sender_),m_delfiles(false) {

		}
		//只有直接传入boost::shared_array<char>才不进行拷贝
		void addData(const std::string& dataTag, const boost::shared_array<char>& buf_, uint32_t buf_size_) {
			map_datas[dataTag] = std::move(ytlib::shared_buf(buf_, buf_size_));
		}
		void addData(const std::string& dataTag, const char* buf_, uint32_t buf_size_) {
			ytlib::shared_buf tmpbuf(buf_size_);
			memcpy(tmpbuf.buf.get(), buf_, buf_size_);
			map_datas[dataTag] = std::move(tmpbuf);
		}
		void addData(const std::string& dataTag, const std::string& data) {
			ytlib::shared_buf tmpbuf(data.size());
			memcpy(tmpbuf.buf.get(), data.c_str(), tmpbuf.buf_size);
			map_datas[dataTag] = std::move(tmpbuf);
		}

		std::string m_name;//数据名称
		std::string m_sender;//数据发送者（插件名称）

		bool m_delfiles;//发送完成后是否删除文件
		std::map<std::string, ytlib::shared_buf> map_datas;//数据,最大支持255个
		std::map<std::string, std::string> map_files;//文件,最大支持255个
		
	};
	//用于传输一个消息
	class rpsfEvent {
		T_CLASS_SERIALIZE(&m_name&m_sender)
	public:
		rpsfEvent() {}
		rpsfEvent(const std::string& name_, const std::string& sender_) :
			m_name(name_), m_sender(sender_){
		}
		void setEventMsg(const char* buf_, uint32_t buf_size_) {
			msgbuf.buf_size = buf_size_;
			msgbuf.buf = boost::shared_array<char>(new char[buf_size_]);
			memcpy(msgbuf.buf.get(), buf_, buf_size_);
		}
		void setEventMsg(const std::string& eventmsg) {
			msgbuf.buf_size = eventmsg.size();
			msgbuf.buf = boost::shared_array<char>(new char[msgbuf.buf_size]);
			memcpy(msgbuf.buf.get(), eventmsg.c_str(), msgbuf.buf_size);
		}

		std::string m_name;//数据名称
		std::string m_sender;//数据发送者（插件名称）

		ytlib::shared_buf msgbuf;

	};
	//用于RPC调用
	class rpsfRpc {

	public:


	};
	//用于RPC返回结果
	class rpsfRRpc {

	public:

	};
	//用于系统消息
	class rpsfSys {

	public:

	};


	//最底层数据包格式
	class rpsfMsg {
		T_CLASS_SERIALIZE(&m_srcAddr&m_msgType&emcy&p_rpsfData&p_rpsfEvent&p_rpsfRpc&p_rpsfRRpc&p_rpsfSys)
	public:
		enum MsgType{
			RPSF_DATA,
			RPSF_EVENT,
			RPSF_RPC,
			RPSF_RRPC,
			RPSF_SYS
		};
		rpsfMsg(){}
		rpsfMsg(uint32_t myid_, MsgType type_, bool is_emcy = false) :
			m_srcAddr(myid_), emcy(is_emcy), m_msgType(type_) {	}

		uint32_t m_srcAddr;//消息源框架id
		MsgType m_msgType;//消息类型
		bool emcy;//是否为立即处理

		//以boost::shared_ptr包装要使用的上层对象。一个shared_ptr只会增加2字节（bin模型）或3字节（text模式）
		boost::shared_ptr<rpsfData> p_rpsfData;
		boost::shared_ptr<rpsfEvent> p_rpsfEvent;
		boost::shared_ptr<rpsfRpc> p_rpsfRpc;
		boost::shared_ptr<rpsfRRpc> p_rpsfRRpc;
		boost::shared_ptr<rpsfSys> p_rpsfSys;
		
	};
	typedef ytlib::DataPackage<rpsfMsg> rpsfDataPackage;

}


