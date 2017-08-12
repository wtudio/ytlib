#pragma once
#include <ytlib/NetTools/TcpNetUtil.h>
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <boost/date_time/posix_time/posix_time.hpp>  


namespace rpsf {

	//本文件中定义一些数据包格式。其中需要给外界提供的数据包需要另外做接口

	enum MsgType {
		RPSF_DATA = 0,
		RPSF_EVENT = 1,
		RPSF_RPC = 2,
		RPSF_RRPC = 3,
		RPSF_SYS = 4,

		RPSF_HANDLETYPE_ORDER = 0,
		RPSF_HANDLETYPE_UNORDER = 16,
		RPSF_HANDLETYPE_EMCY = 32
	};


	//用于传输大量数据
	class rpsfDataMsg {
		//只需要序列化前几个
		T_CLASS_SERIALIZE(&m_name&m_sender)
	public:
		rpsfDataMsg() {}
		rpsfDataMsg(const std::string& name_, const std::string& sender_) :m_name(name_), m_sender(sender_),m_delfiles(false),
			m_msgType(MsgType::RPSF_DATA| MsgType::RPSF_HANDLETYPE_UNORDER){

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
		void addData(const std::string& dataTag, const std::string& data_) {
			ytlib::shared_buf tmpbuf(data_.size());
			memcpy(tmpbuf.buf.get(), data_.c_str(), tmpbuf.buf_size);
			map_datas[dataTag] = std::move(tmpbuf);
		}

		std::string m_name;//数据名称
		std::string m_sender;//数据发送者（插件名称）


		std::map<std::string, ytlib::shared_buf> map_datas;//数据,最大支持255个
		std::map<std::string, std::string> map_files;//文件,最大支持255个
		
		uint8_t m_msgType;
		bool m_delfiles;//发送完成后是否删除文件
	};
	//用于传输单条消息
	class rpsfEventMsg {
		T_CLASS_SERIALIZE(&m_name&m_sender&m_eventTime)
	public:
		rpsfEventMsg() {}
		rpsfEventMsg(const std::string& name_, const std::string& sender_) :m_name(name_), m_sender(sender_), 
			m_msgType(MsgType::RPSF_EVENT | MsgType::RPSF_HANDLETYPE_ORDER) {
		}
		void setEventMsg(const boost::shared_array<char>& buf_, uint32_t buf_size_) {
			map_datas["e"] = std::move(ytlib::shared_buf(buf_, buf_size_));
		}

		void setEventMsg(const char* buf_, uint32_t buf_size_) {
			ytlib::shared_buf tmpbuf(buf_size_);
			memcpy(tmpbuf.buf.get(), buf_, buf_size_);
			map_datas["e"] = std::move(tmpbuf);
		}
		void setEventMsg(const std::string& eventmsg) {
			ytlib::shared_buf tmpbuf(eventmsg.size());
			memcpy(tmpbuf.buf.get(), eventmsg.c_str(), tmpbuf.buf_size);
			map_datas["e"] = std::move(tmpbuf);
		}

		void setEventTime() {
			boost::posix_time::ptime tnow = boost::posix_time::microsec_clock::universal_time();
			memcpy(&m_eventTime, &tnow, 8);
		}
		std::string getEventTime() {
			boost::posix_time::ptime pt;
			memcpy(&pt, &m_eventTime, 8);
			return boost::posix_time::to_iso_string(pt);
		}

		std::string m_name;//数据名称
		std::string m_sender;//数据发送者（插件名称）
		uint64_t m_eventTime;

		std::map<std::string, ytlib::shared_buf> map_datas;//数据,最大支持255个

		uint8_t m_msgType;
	};
	//用于RPC调用。参数支持文件
	class rpsfRpc {
		T_CLASS_SERIALIZE(&m_service&m_fun&m_rpcID)
	public:
		rpsfRpc(){}
		rpsfRpc(const std::string& service_, const std::string& fun_) :
			m_service(service_), m_fun(fun_), m_msgType(MsgType::RPSF_RPC | MsgType::RPSF_HANDLETYPE_UNORDER) {

		}

		std::string m_service;//远程插件的名称
		std::string m_fun;//远程插件的方法
		uint32_t m_rpcID;//rpc的编号。应保证一个框架内的rpc编号互不一样

		uint32_t m_timeout;//超时时间。0表示一直等待

		std::map<std::string, ytlib::shared_buf> map_datas;//数据,最大支持255个
		std::map<std::string, std::string> map_files;//文件,最大支持255个

		uint8_t m_msgType;
		bool m_delfiles;//发送完成后是否删除文件
	};
	//用于RPC返回结果
	class rpsfRRpc {

	public:
		uint32_t m_rpcID;//回应的rpc的编号
		uint32_t m_errorCode;//错误码

	};

	//用于系统消息。不提供给插件调用，一般用于传递各种结构体
	class rpsfSys {
		T_CLASS_SERIALIZE(&m_sysMsgType)
	public:
		//系统消息类型
		enum {
			RPSF_SYS_NEWNODE
		};
		rpsfSys() :m_msgType(MsgType::RPSF_SYS | MsgType::RPSF_HANDLETYPE_EMCY) {

		}

		uint8_t m_sysMsgType;//消息类型


		std::map<std::string, ytlib::shared_buf> map_datas;//数据,最大支持255个

		uint8_t m_msgType;
	};


	//最底层数据包格式
	class rpsfMsg {
		T_CLASS_SERIALIZE(&m_srcAddr&m_msgType&emcy&p_rpsfData&p_rpsfEvent&p_rpsfRpc&p_rpsfRRpc&p_rpsfSys)
	public:

		rpsfMsg(){}
		rpsfMsg(uint32_t myid_, uint8_t type_) :m_srcAddr(myid_), m_msgType(type_) {}

		uint32_t m_srcAddr;//消息源框架id
		uint8_t m_msgType;//消息类型。因为要支持序列化所以类型为int


		//以boost::shared_ptr包装要使用的上层对象。一个shared_ptr只会增加2字节（bin模型）或3字节（text模式）
		boost::shared_ptr<rpsfDataMsg> p_rpsfData;
		boost::shared_ptr<rpsfEventMsg> p_rpsfEvent;
		boost::shared_ptr<rpsfRpc> p_rpsfRpc;
		boost::shared_ptr<rpsfRRpc> p_rpsfRRpc;
		boost::shared_ptr<rpsfSys> p_rpsfSys;
		
	};
	typedef ytlib::DataPackage<rpsfMsg> rpsfDataPackage;

}


