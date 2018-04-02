#pragma once
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <ytrpsf/Plugin_Bus_Interface.h>
#include <ytlib/SupportTools/Serialize.h>

namespace rpsf {

	//本文件中实现各种数据包
	enum MsgType {
		RPSF_DATA = 0,
		RPSF_RPC = 1,
		RPSF_RRPC = 2,
		RPSF_SYS = 3
	};

	//最底层数据包格式。快速数据： event：数据，RRPC：错误信息
	class rpsfMsg {
		T_CLASS_SERIALIZE(&m_srcAddr&m_msgType&m_handleType)
	public:
		rpsfMsg():m_srcAddr(0),m_delfiles(false){}

		uint32_t m_srcAddr;//消息源框架id。0代表本地
		MsgType m_msgType;//消息类型。因为要支持序列化所以类型为uint8
		HandleType m_handleType;//处理方式

		std::set<uint32_t> m_pushList;//推送名单
		bool m_delfiles;//发送完成后是否删除文件
	};
	typedef ytlib::DataPackage<rpsfMsg> rpsfDataPackage;
	typedef std::shared_ptr<rpsfDataPackage> rpsfPackagePtr;

	//将各种类型的消息包打包为一个rpsfPackagePtr。同时原消息包将不可用
	static rpsfPackagePtr setBaseMsgToPackage(rpsfPackage& m_) {
		rpsfPackagePtr package = std::make_shared<rpsfDataPackage>();
		package->map_datas = std::move(m_.m_mapDatas);
		package->map_files = std::move(m_.m_mapFiles);
		package->obj.m_handleType = m_.m_handleType;
		package->obj.m_delfiles = m_.m_bDelFiles;
		return std::move(package);
	}
	static rpsfPackagePtr setMsgToPackage(rpsfRpcArgs& m_) {
		rpsfPackagePtr package = setBaseMsgToPackage(m_);
		package->obj.m_msgType = MsgType::RPSF_RPC;
		package->quick_data = ytlib::sharedBuf(m_.m_service);
		return std::move(package);
	}
	static rpsfPackagePtr setMsgToPackage(rpsfData& m_) {
		rpsfPackagePtr package = setBaseMsgToPackage(m_);
		package->obj.m_msgType = MsgType::RPSF_DATA;
		package->quick_data = ytlib::sharedBuf(m_.m_dataName);
		return std::move(package);
	}
	static rpsfPackagePtr setMsgToPackage(rpsfRpcResult& m_, uint32_t dst) {
		rpsfPackagePtr package = setBaseMsgToPackage(m_);
		package->obj.m_msgType = MsgType::RPSF_RRPC;
		//BusErr总数小于256个.因为考虑跨平台时的大小端问题
		package->quick_data = ytlib::sharedBuf(5 + static_cast<uint32_t>(m_.m_errMsg.size()));
		ytlib::set_buf_from_num(package->quick_data.buf.get(), dst);
		package->quick_data.buf[4] = static_cast<uint8_t>(m_.m_rpcErr);
		memcpy(package->quick_data.buf.get() + 5, m_.m_errMsg.c_str(), m_.m_errMsg.size());
		return std::move(package);
	}

	//将rpsfPackagePtr解包为各种类型的消息包，同时原包将不可用
	static void getBaseMsgFromPackage(rpsfPackagePtr& package_, rpsfPackage& m_) {
		m_.m_mapDatas = std::move(package_->map_datas);
		m_.m_mapFiles = std::move(package_->map_files);
		m_.m_handleType = package_->obj.m_handleType;
	}
	static void getMsgFromPackage(rpsfPackagePtr& package_, rpsfRpcArgs& m_) {
		getBaseMsgFromPackage(package_, m_);
		m_.m_service.assign(package_->quick_data.buf.get(), package_->quick_data.buf_size);
	}
	static void getMsgFromPackage(rpsfPackagePtr& package_, rpsfData& m_) {
		getBaseMsgFromPackage(package_, m_);
		m_.m_dataName.assign(package_->quick_data.buf.get(), package_->quick_data.buf_size);
	}
	static void getMsgFromPackage(rpsfPackagePtr& package_, rpsfRpcResult& m_) {
		getBaseMsgFromPackage(package_, m_);
		m_.m_rpcErr = static_cast<BusErr>(static_cast<uint8_t>(package_->quick_data.buf[4]));
		m_.m_errMsg.assign(package_->quick_data.buf.get() + 5, package_->quick_data.buf_size - 5);
	}

	//从rpsfPackagePtr提取出各个消息的key
	static std::string getDataNameFromPackage(const rpsfPackagePtr& package_) {
		return std::string(package_->quick_data.buf.get(), package_->quick_data.buf_size);
	}
	static std::string getServiceFromPackage(const rpsfPackagePtr& package_) {
		return std::string(package_->quick_data.buf.get(), package_->quick_data.buf_size);
	}
	static uint32_t getRrpcDstFromPackage(const rpsfPackagePtr& package_ ) {
		return ytlib::get_num_from_buf(package_->quick_data.buf.get());
	}

	//使用高性能序列化方法封装的序列化到sharedBuf的方法。二进制序列化
	template<class T>
	bool Serialize(const T& obj, ytlib::sharedBuf& data) {
		try {
			char* p;
			size_t len;
			SERIALIZE_INIT;
			SERIALIZE(obj, p, len);
			data.buf = boost::shared_array<char>(new char[len]);
			memcpy(data.buf.get(), p, len);
			return true;
		}
		catch (const std::exception& e) {
			std::cout << "Serialize failed:" << e.what() << std::endl;
			return false;
		}
	}
	template<class T>
	bool Deserialize_f(const T& obj, ytlib::sharedBuf& data) {
		try {
			DESERIALIZE_INIT;
			DESERIALIZE(obj, data.buf.get(), data.buf_size);
			return true;
		}
		catch (const std::exception& e) {
			std::cout << "Serialize failed:" << e.what() << std::endl;
			return false;
		}
	}

}


