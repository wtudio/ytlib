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
		void OnData(const rpsf::rpsfData& data_) {

		}
		void OnEvent(const rpsf::rpsfEvent& event_) {

		}
		rpsf::rpsfResult Invoke(const rpsf::rpsfCallArgs& callArgs_) {
			rpsf::rpsfResult result_;
			if (callArgs_.getFunName() == "testRPC") {
				//将参数解包，调用真正的处理函数并将结果打包返回
				para2 p_;
				callArgs_.getData("buf", p_.buf_, p_.buf_size_);
				callArgs_.getFile("f", p_.file1);
				result1 r_;
				testRPC(p_, r_);
				
				result_.addData("msg", r_.remsg);
				result_.addFile("f", r_.refile);
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