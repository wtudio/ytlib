#pragma once
#include <map>
#include <set>
#include <boost/shared_array.hpp>

namespace rpsf {
	//本文件提供bus的接口和一些数据包的定义
	enum HandleType {
		RPSF_ORDER,
		RPSF_UNORDER,
		RPSF_EMCY
	};

	class rpsfPackage {
	public:
		rpsfPackage();

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

		void setIfDelFiles(bool delfiles_);
		bool getIfDelFiles() const;

		void setHandleType(HandleType type_);
		HandleType getHandleType() const;
	protected:
		std::map<std::string,>
	};

	//数据包，用来发送数据
	class rpsfData : public rpsfPackage {
	public:
		explicit rpsfData(const std::string& name_, const std::string& sender_);
		std::string getDataName() const;
		std::string getSender() const;

	};
	//事件包，用来发布事件
	class rpsfEvent : private rpsfPackage {
	public:
		explicit rpsfEvent(const std::string& name_, const std::string& sender_);
		std::string getEventName() const;
		std::string getSender() const;
		void setEventTime();
		std::string getEventTime() const;

		void setEventMsg(const boost::shared_array<char>& buf_, uint32_t buf_size_);
		void setEventMsg(const char* buf_, uint32_t buf_size_);
		void setEventMsg(const std::string& eventmsg);
		void clearEventMsg();
		void getEventMsg(boost::shared_array<char>& buf_, uint32_t& buf_size_) const;

	};
	//RPC调用包，用来存储RPC参数
	class rpsfCallArgs : public rpsfPackage {
	public:
		explicit rpsfCallArgs(const std::string& service_, const std::string& fun_, uint32_t timeout = 0);
		std::string getServiceName() const;
		std::string getFunName() const;
		uint32_t getTimeOut() const;

	};
	//RPC调用返回包，用来存储RPC结果
	class rpsfResult : public rpsfPackage {
	public:
		explicit rpsfResult();
		operator bool() const;
		std::string getErrorInfo() const;

		void setErrorInfo(const std::string& err_);//RPC执行方填入错误信息。空字符串表示无错误
	};

	//对bus的接口封装，提供给插件使用
	class IBus {
	public:
		IBus(void* p_);
		//订阅、取消订阅数据、事件
		bool SubscribeData(const std::string& pluginName_, const std::set<std::string>& dataNames_);
		bool UnsubscribeData(const std::string& pluginName_, const std::set<std::string>& dataName_);
		bool SubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_);
		bool UnsubscribeEvent(const std::string& pluginName_, const std::set<std::string>& eventName_);

		
		rpsfResult Invoke(const rpsfCallArgs& callArgs_);//RPC接口
		bool PublishEvent(const rpsfEvent& event_);//发布事件
		bool PublishData(const rpsfData& data_);//发布数据

		//获取接收、发送路径
		std::string GetSendFilePath();
		std::string GetRecvFilePath();

	private:
		void * p;
	};

}