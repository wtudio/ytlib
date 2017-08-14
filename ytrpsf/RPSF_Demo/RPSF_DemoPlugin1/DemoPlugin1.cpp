#include <ytrpsf/RPSF_Demo/RPSF_DemoPlugin1/DemoPlugin1_Client.h>

namespace {
	class DemoPlugin1 : public rpsf::IPlugin {
	public:
		//名称和服务需要在插件构造时确定下来并且不能改变
		DemoPlugin1(rpsf::IBus* pBus_) :
			rpsf::IPlugin(pBus_, "DemoPlugin1", std::set<std::string>{"testRPC"}) {
		}
		~DemoPlugin1() {}
		bool Start(std::map<std::string, std::string>& params_) {

			return true;
		}
		void Stop(void) {

		}
		void OnData(const rpsf::rpsfData& data_, const std::string& dataTime_) {

		}
		rpsf::rpsfRpcResult Invoke(const rpsf::rpsfRpcArgs& callArgs_) {
			rpsf::rpsfRpcResult result_;
			if (callArgs_.m_fun == "testRPC") {
				//将参数解包，调用真正的处理函数并将结果打包返回
				para2 p_;
				std::map<std::string, std::pair<boost::shared_array<char>, uint32_t>>::const_iterator itr = callArgs_.m_mapDatas.find("buf");
				if (itr == callArgs_.m_mapDatas.end()) {
					result_.m_errMsg = "err args";
					return result_;
				}
				p_.buf_ = itr->second.first;
				p_.buf_size_ = itr->second.second;
				std::map<std::string, std::string>::const_iterator itrf = callArgs_.m_mapFiles.find("f");
				if (itrf == callArgs_.m_mapFiles.end()) {
					result_.m_errMsg = "err args";
					return result_;
				}
				p_.file1 = itrf->second;
				result1 r_;
				testRPC(p_, r_);
				result_.addData("msg", r_.remsg);
				result_.m_mapFiles["f"] = r_.refile;
			}
			return result_;
		}

		bool testRPC(const para2& p_, result1& r_) {
			return true;
		}

	};
}
#if defined(_WIN32)
#	define _DECLSPEC_EXPORT __declspec(dllexport)
#else
#	define _DECLSPEC_EXPORT
#endif

extern "C" _DECLSPEC_EXPORT rpsf::IPlugin* CreatePlugin(rpsf::IBus* pBus_) {
	return new DemoPlugin1(pBus_);
}