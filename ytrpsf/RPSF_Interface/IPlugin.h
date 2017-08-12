#pragma once
#include <map>
#include <set>
#include <boost/noncopyable.hpp>
#include <boost/shared_array.hpp>

namespace rpsf {
	//本文件提供给插件开发方

	//用户创建的插件需要实现此接口
#ifndef CREATE_PLUGIN_STRING
#define CREATE_PLUGIN_STRING CL_TEXT("CreatePlugin")
#endif

	class rpsfData : boost::noncopyable{
	public:
		explicit rpsfData(const std::string& name_, const std::string& sender_);
		std::string getDataName() const;
		std::string getSender() const;
		
		void addData(const std::string& dataTag, const boost::shared_array<char>& buf_, uint32_t buf_size_);
		void addData(const std::string& dataTag, const char* buf_, uint32_t buf_size_);
		void addData(const std::string& dataTag, const std::string& data_);
		void delData(const std::string& dataTag);
		void clearDatas();

		void addFile(const std::string& fileTag, const std::string& filePath_);
		void delFile(const std::string& dataTag);
		void clearFiles();

		bool getData(const std::string& dataTag, boost::shared_array<char>& buf_, uint32_t& buf_size_) const;
		std::set<std::string> getDataList() const;
		bool getFile(const std::string& fileTag, std::string& filePath_) const;
		std::set<std::string> getFileList() const;

	private:
		void* p;
	};
	class rpsfEvent : boost::noncopyable {
	public:
		explicit rpsfEvent(const std::string& name_, const std::string& sender_);
		std::string getDataName() const;
		std::string getSender() const;
		std::string getEventTime() const;

		void setEventMsg(const boost::shared_array<char>& buf_, uint32_t buf_size_);
		void setEventMsg(const char* buf_, uint32_t buf_size_);
		void setEventMsg(const std::string& eventmsg);
		void clearEventMsg();

		void getEventMsg(boost::shared_array<char>& buf_, uint32_t& buf_size_) const;
	private:
		void* p;
	};
	class rpsfCallArgs : boost::noncopyable {
	public:
		explicit rpsfCallArgs(const std::string& service_, const std::string& fun_, uint32_t timeout = 0);
		std::string getServiceName() const;
		std::string getFunName() const;
		uint32_t getTimeOut() const;

		void addCallArgsData(const std::string& dataTag, const boost::shared_array<char>& buf_, uint32_t buf_size_);
		void addCallArgsData(const std::string& dataTag, const char* buf_, uint32_t buf_size_);
		void addCallArgsData(const std::string& dataTag, const std::string& data_);
		void delCallArgsData(const std::string& dataTag);
		void clearCallArgsDatas();

		void addCallArgsFile(const std::string& fileTag, const std::string& filePath_);
		void delCallArgsFile(const std::string& dataTag);
		void clearCallArgsFiles();

		bool getCallArgsData(const std::string& dataTag, boost::shared_array<char>& buf_, uint32_t& buf_size_) const;
		std::set<std::string> getCallArgsDataList() const;
		bool getCallArgsFile(const std::string& fileTag, std::string& filePath_) const;
		std::set<std::string> getCallArgsFileList() const;

	private:
		void* p;
	};
	class rpsfResult : boost::noncopyable {
	public:
		explicit rpsfResult();
		bool is_success() const;
		std::string getErrorInfo() const;

		void addResultData(const std::string& dataTag, const boost::shared_array<char>& buf_, uint32_t buf_size_);
		void addResultData(const std::string& dataTag, const char* buf_, uint32_t buf_size_);
		void addResultData(const std::string& dataTag, const std::string& data_);
		void delResultData(const std::string& dataTag);
		void clearResultDatas();

		void addResultFile(const std::string& fileTag, const std::string& filePath_);
		void delResultFile(const std::string& dataTag);
		void clearResultFiles();

		bool getResultData(const std::string& dataTag, boost::shared_array<char>& buf_, uint32_t& buf_size_) const;
		std::set<std::string> getResultDataList() const;
		bool getResultFile(const std::string& fileTag, std::string& filePath_) const;
		std::set<std::string> getResultFileList() const;

	private:
		void* p;
	};

	//对bus的接口封装，提供给插件使用
	class IBus {
	public:
		IBus(void* p_):p(p_){}
		bool SubscribeData(IPlugin* plugin_, const std::string& dataName_);
		bool UnsubscribeData(IPlugin* plugin_, const std::string& dataName_);
		bool SubscribeEvent(IPlugin* plugin_, const std::string& eventName_);
		bool UnsubscribeEvent(IPlugin* plugin_, const std::string& eventName_);

		void Invoke(const rpsfCallArgs& callArgs_, rpsfResult& result_);
		bool PublishEvent(const rpsfEvent& event_);
		bool PublishData(const rpsfData& data_);

		std::string GetSendFilePath();
		std::string GetRecvFilePath();

	private:
		void * p;
	};

	//plugin的虚基类，直接提供给用户进行开发。用户需要继承它
	class IPlugin {
	public:
		//名称和服务需要在插件构造时确定下来并且不能改变
		IPlugin(IBus* pBus_, const std::string& name_, const std::set<std::string>& funs_) :
			pBus(pBus_) , name(name_), funs(funs_){	}
		virtual ~IPlugin() {}
		virtual bool Start(std::map<std::string, std::string>& params_) = 0;
		virtual void Stop(void) = 0;
		virtual void OnData(const rpsfData& data_) = 0;
		virtual void OnEvent(const rpsfEvent& event_) = 0;
		virtual void Invoke(const rpsfCallArgs& callArgs_, rpsfResult& result_) = 0;

		const std::string name;
		const std::set<std::string> funs;
	private:
		IBus* pBus;
	};








}