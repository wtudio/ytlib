#pragma once
#include <ytlib/NetTools/TcpNetUtil.h>
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <ytrpsf/RPSF_Interface/IBus.h>
#include <boost/date_time/posix_time/posix_time.hpp>  

namespace rpsf {

	//本文件中实现各种数据包
	enum MsgType {
		RPSF_DATA = 0,
		RPSF_EVENT = 1 * 16,
		RPSF_RPC = 2 * 16,
		RPSF_RRPC = 3 * 16,
		RPSF_SYS = 4 * 16
	};

	//最底层数据包格式。快速数据： event：数据，RRPC：错误信息
	class rpsfMsg {
		T_CLASS_SERIALIZE(&m_srcAddr&m_msgType&s1&s2&i1&i2)
	public:
		rpsfMsg():m_delfiles(false){}

		uint32_t m_srcAddr;//消息源框架id
		uint8_t m_msgType;//消息类型。因为要支持序列化所以类型为int
		//一些快速访问数据可以放在这
		std::string s1;//data\event：数据名称，RPC：service名称
		std::string s2;//data\event：发送者，RPC：fun名称
		uint32_t i1;//event：时间高32位，RPC\RRPC：rpcID 
		uint32_t i2;//event：时间低32位，RPC：timeout，RRPC错误码

		bool m_delfiles;//发送完成后是否删除文件
	};
	typedef ytlib::DataPackage<rpsfMsg> rpsfDataPackage;

	//--------------------------------rpsfPackage--------------------------------
#define RPSFPACKAGE(p) static_cast<rpsfDataPackage*>(p.get())

	rpsfPackage::rpsfPackage() {
		p = std::shared_ptr<rpsfDataPackage>(new rpsfDataPackage());
	}
	rpsfPackage rpsfPackage::getCopy() const {
		rpsfPackage re;
		RPSFPACKAGE(p)->m_sysMutex.lock_shared();
		RPSFPACKAGE(re.p)->obj = RPSFPACKAGE(p)->obj;
		RPSFPACKAGE(re.p)->quick_data = RPSFPACKAGE(p)->quick_data.getCopy();
		RPSFPACKAGE(p)->m_sysMutex.unlock_shared();

		RPSFPACKAGE(p)->m_datasMutex.lock_shared();
		for (std::map<std::string, ytlib::shared_buf>::iterator itr = RPSFPACKAGE(p)->map_datas.begin();
			itr != RPSFPACKAGE(p)->map_datas.end(); ++itr) {
			RPSFPACKAGE(re.p)->map_datas[itr->first] = itr->second.getCopy();
		}
		RPSFPACKAGE(p)->m_datasMutex.unlock_shared();

		RPSFPACKAGE(p)->m_filesMutex.lock_shared();
		RPSFPACKAGE(re.p)->map_files = RPSFPACKAGE(p)->map_files;
		RPSFPACKAGE(p)->m_filesMutex.unlock_shared();
	}
	void rpsfPackage::addData(const std::string& dataTag, const boost::shared_array<char>& buf_, uint32_t buf_size_) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_datasMutex);
		RPSFPACKAGE(p)->map_datas[dataTag] = std::move(ytlib::shared_buf(buf_, buf_size_));
	}
	void rpsfPackage::addData(const std::string& dataTag, const char* buf_, uint32_t buf_size_) {
		ytlib::shared_buf tmpbuf(buf_size_);
		memcpy(tmpbuf.buf.get(), buf_, buf_size_);
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_datasMutex);
		RPSFPACKAGE(p)->map_datas[dataTag] = std::move(tmpbuf);
	}
	void rpsfPackage::addData(const std::string& dataTag, const std::string& data_) {
		ytlib::shared_buf tmpbuf(data_.size());
		memcpy(tmpbuf.buf.get(), data_.c_str(), tmpbuf.buf_size);
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_datasMutex);
		RPSFPACKAGE(p)->map_datas[dataTag] = std::move(tmpbuf);
	}
	void rpsfPackage::delData(const std::string& dataTag) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_datasMutex);
		std::map<std::string, ytlib::shared_buf>::iterator itr = RPSFPACKAGE(p)->map_datas.find(dataTag);
		if (itr != RPSFPACKAGE(p)->map_datas.end()) RPSFPACKAGE(p)->map_datas.erase(itr);
	}
	void rpsfPackage::clearDatas() {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_datasMutex);
		RPSFPACKAGE(p)->map_datas.clear();
	}

	void rpsfPackage::addFile(const std::string& fileTag, const std::string& filePath_) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_filesMutex);
		RPSFPACKAGE(p)->map_files[fileTag] = filePath_;
	}
	void rpsfPackage::delFile(const std::string& dataTag) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_filesMutex);
		std::map<std::string, std::string>::iterator itr = RPSFPACKAGE(p)->map_files.find(dataTag);
		if (itr != RPSFPACKAGE(p)->map_files.end()) RPSFPACKAGE(p)->map_files.erase(itr);
	}
	void rpsfPackage::clearFiles() {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_filesMutex);
		RPSFPACKAGE(p)->map_files.clear();
	}

	bool rpsfPackage::getData(const std::string& dataTag, boost::shared_array<char>& buf_, uint32_t& buf_size_) const {
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_datasMutex);
		std::map<std::string, ytlib::shared_buf>::iterator itr = RPSFPACKAGE(p)->map_datas.find(dataTag);
		if (itr != RPSFPACKAGE(p)->map_datas.end()) {
			buf_ = itr->second.buf;
			buf_size_ = itr->second.buf_size;
			return true;
		}
		return false;
	}
	std::set<std::string> rpsfPackage::getDataList() const {
		std::set<std::string> re;
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_datasMutex);
		for (std::map<std::string, ytlib::shared_buf>::iterator itr = RPSFPACKAGE(p)->map_datas.begin();
			itr != RPSFPACKAGE(p)->map_datas.end(); ++itr) {
			re.insert(itr->first);
		}
		return std::move(re);
	}
	bool rpsfPackage::getFile(const std::string& fileTag, std::string& filePath_) const {
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_filesMutex);
		std::map<std::string, std::string>::iterator itr = RPSFPACKAGE(p)->map_files.find(fileTag);
		if (itr != RPSFPACKAGE(p)->map_files.end()) {
			filePath_ = itr->second;
			return true;
		}
		return false;
	}
	std::set<std::string> rpsfPackage::getFileList() const {
		std::set<std::string> re;
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_filesMutex);
		for (std::map<std::string, std::string>::iterator itr = RPSFPACKAGE(p)->map_files.begin();
			itr != RPSFPACKAGE(p)->map_files.end(); ++itr) {
			re.insert(itr->first);
		}
		return std::move(re);
	}
	void rpsfPackage::setIfDelFiles(bool delfiles_) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->obj.m_delfiles = delfiles_;
	}
	bool rpsfPackage::getIfDelFiles() const {
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		return RPSFPACKAGE(p)->obj.m_delfiles;
	}
	void rpsfPackage::setHandleType(handleType type_) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->obj.m_msgType = static_cast<uint8_t>(type_);
	}
	handleType rpsfPackage::getHandleType() const {
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		return static_cast<handleType>(RPSFPACKAGE(p)->obj.m_msgType);
	}
	//--------------------------------rpsfData--------------------------------
	rpsfData::rpsfData(const std::string& name_, const std::string& sender_) {
		RPSFPACKAGE(p)->obj.s1 = name_;
		RPSFPACKAGE(p)->obj.s2 = sender_;
	}
	std::string rpsfData::getDataName() const {
		return RPSFPACKAGE(p)->obj.s1;
	}
	std::string rpsfData::getSender() const {
		return RPSFPACKAGE(p)->obj.s2;
	}
	//--------------------------------rpsfEvent--------------------------------
	rpsfEvent::rpsfEvent(const std::string& name_, const std::string& sender_) {
		RPSFPACKAGE(p)->obj.s1 = name_;
		RPSFPACKAGE(p)->obj.s2 = sender_;
	}
	std::string rpsfEvent::getEventName() const {
		return RPSFPACKAGE(p)->obj.s1;
	}
	std::string rpsfEvent::getSender() const {
		return RPSFPACKAGE(p)->obj.s2;
	}
	void rpsfEvent::setEventTime() {
		boost::posix_time::ptime pt = boost::posix_time::microsec_clock::universal_time();
		uint64_t n;
		memcpy(&n, &pt, 8);
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->obj.i2 = n % (65536 * 65536);
		RPSFPACKAGE(p)->obj.i1 = n / (65536 * 65536);
	}
	std::string rpsfEvent::getEventTime() const {
		RPSFPACKAGE(p)->m_sysMutex.lock_shared();
		uint64_t n = RPSFPACKAGE(p)->obj.i2 + static_cast<uint64_t>(RPSFPACKAGE(p)->obj.i1)*(65536 * 65536);
		RPSFPACKAGE(p)->m_sysMutex.unlock_shared();
		boost::posix_time::ptime pt;
		memcpy(&pt, &n, 8);
		return boost::posix_time::to_iso_string(pt);
	}
	void rpsfEvent::setEventMsg(const boost::shared_array<char>& buf_, uint32_t buf_size_) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->quick_data.buf = buf_;
		RPSFPACKAGE(p)->quick_data.buf_size = buf_size_;
	}
	void rpsfEvent::setEventMsg(const char* buf_, uint32_t buf_size_) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->quick_data.buf_size = buf_size_;
		RPSFPACKAGE(p)->quick_data.buf= boost::shared_array<char>(new char[buf_size_]);
		memcpy(RPSFPACKAGE(p)->quick_data.buf.get(), buf_, buf_size_);
	}
	void rpsfEvent::setEventMsg(const std::string& eventmsg) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->quick_data.buf_size = eventmsg.size();
		RPSFPACKAGE(p)->quick_data.buf = boost::shared_array<char>(new char[RPSFPACKAGE(p)->quick_data.buf_size]);
		memcpy(RPSFPACKAGE(p)->quick_data.buf.get(), eventmsg.c_str(), RPSFPACKAGE(p)->quick_data.buf_size);
	}
	void rpsfEvent::clearEventMsg() {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->quick_data.buf = boost::shared_array<char>();
		RPSFPACKAGE(p)->quick_data.buf_size = 0;
	}
	void rpsfEvent::getEventMsg(boost::shared_array<char>& buf_, uint32_t& buf_size_) const {
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		buf_ = RPSFPACKAGE(p)->quick_data.buf;
		buf_size_ = RPSFPACKAGE(p)->quick_data.buf_size;
	}
	//--------------------------------rpsfCallArgs--------------------------------
	rpsfCallArgs::rpsfCallArgs(const std::string& service_, const std::string& fun_, uint32_t timeout) {
		RPSFPACKAGE(p)->obj.s1 = service_;
		RPSFPACKAGE(p)->obj.s2 = fun_;
		RPSFPACKAGE(p)->obj.i2 = timeout;
	}
	std::string rpsfCallArgs::getServiceName() const {
		return RPSFPACKAGE(p)->obj.s1;
	}
	std::string rpsfCallArgs::getFunName() const {
		return RPSFPACKAGE(p)->obj.s2;
	}
	uint32_t rpsfCallArgs::getTimeOut() const {
		return RPSFPACKAGE(p)->obj.i2;
	}
	//--------------------------------rpsfCallArgs--------------------------------
	rpsfResult::rpsfResult() {
		RPSFPACKAGE(p)->obj.i2 = 0;
	}
	rpsfResult::operator bool() const {
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		return (RPSFPACKAGE(p)->obj.i2 == 0) && (RPSFPACKAGE(p)->quick_data.buf_size == 0);
	}
	std::string rpsfResult::getErrorInfo() const {
		std::shared_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		return std::string(RPSFPACKAGE(p)->quick_data.buf.get(), RPSFPACKAGE(p)->quick_data.buf_size);
	}

	void rpsfResult::setErrorInfo(const std::string& err_) {
		std::unique_lock<std::shared_mutex> lck(RPSFPACKAGE(p)->m_sysMutex);
		RPSFPACKAGE(p)->quick_data.buf_size = err_.size();
		RPSFPACKAGE(p)->quick_data.buf = boost::shared_array<char>(new char[RPSFPACKAGE(p)->quick_data.buf_size]);
		memcpy(RPSFPACKAGE(p)->quick_data.buf.get(), err_.c_str(), RPSFPACKAGE(p)->quick_data.buf_size);
	}
}


