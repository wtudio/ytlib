/**
 * @file AlgProcess.h
 * @brief 算法过程
 * @details 定义了基本的算法过程
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once
#include <ytlib/ProcessManager/ProcessBase.h>
#include <sstream>
#include <map>
#include <boost/thread.hpp>

namespace ytlib
{
	enum AlgRunType	{
		ALG_SYNC,
		ALG_ASYNC
	};

	enum AlgState {
		ALGSTATE_RUNNING =0,
		ALGSTATE_SUCCESS =0,
		ALGSTATE_STOP,
		ALGSTATE_INPUT_FILE_ERR,
		ALGSTATE_COUNT
	};

	const static ytlib::tstring AlgStateMsg[ALGSTATE_COUNT] = {
		T_TEXT("运行正常."),
		T_TEXT("算法已被强制停止."),
		T_TEXT("输入文件错误.")
	};

	//进度回调
	typedef std::function<void(int32_t)> ScheCallback;
	/**
	* @brief 算法类Process
	* 开启一个线程来进行算法计算
	*/
	class AlgProcess : public ProcessBase
	{
	public:
		AlgProcess() :ProcessBase(), m_state(0), m_scheRange(10000){
			registerScheCallback(std::bind(&AlgProcess::defScheCallback, this, std::placeholders::_1));
		}
		virtual ~AlgProcess() {
			if(m_bRunning) stop();
		}
		bool init(const std::map<tstring, tstring>& m) {
			m_mapFiles = m;
			return ProcessBase::init();
		}
		bool init() {
			return ProcessBase::init();
		}

		///主要起示例作用，默认异步运行
		bool start(AlgRunType type_= AlgRunType::ALG_ASYNC) {
			if (!ProcessBase::start()) return false;
			if (type_ == AlgRunType::ALG_ASYNC) {
				mainAlgThread = boost::thread(std::bind(&AlgProcess::mainAlg, this));
			}
			else {
				mainAlg();
			}
			return true;
		}
		///仅在异步模式下有用
		bool stop(int32_t waittime=1000) {
			if (!ProcessBase::stop()) return false;
			//等待 waittime ms，如果还不行就返回false
			if (!mainAlgThread.timed_join(boost::posix_time::millisec(waittime))){
				fLogCallback(T_TEXT("算法流程无法停止!"));
				return false;
			}
			return true;
		}

		bool isRunning() {
			if (!m_bRunning) return false;
			if (mainAlgThread.timed_join(boost::posix_time::millisec(0))) {
				m_bRunning = false;
			}
			return m_bRunning;
		}

		inline int32_t getCurState() {
			return m_state;
		}
		virtual ytlib::tstring getStateMsg() {
			if (m_state >= AlgState::ALGSTATE_COUNT) return T_TEXT("未知错误");
			return AlgStateMsg[m_state];
		}
		inline void registerScheCallback(ScheCallback f) {
			fScheCallback = f;
		}
		bool setFiles(const tstring& filename, const tstring&  filepath){
			if (m_bRunning) {
				fLogCallback(T_TEXT("算法正在运行，无法设置文件."));
				return false;
			}
			m_mapFiles[filename] = filepath;
			return true;
		}
		inline std::map<tstring, tstring> getFiles() const{
			return m_mapFiles;
		}

		inline uint32_t getScheRange() const {
			return m_scheRange;
		}

	protected:
		//暂时不提供暂停功能
		bool pause() { return true; }

		void defScheCallback(int32_t s) {
			tostringstream ss;
			ss << T_TEXT("当前进度:") << s;
			fLogCallback(ss.str());
		}
		ScheCallback fScheCallback;
		uint32_t m_scheRange;//进度值上限。超过进度值意味着算法没有正常结束
		//状态值
		uint32_t m_state;
		//所需文件：名称+目录
		std::map<tstring, tstring> m_mapFiles;//输入的参数文件路径和输出文件路径一块
		//线程
		boost::thread mainAlgThread;
		
		//只需要继承此方法就行了
		//因为要提供类内接口支持，所以不能改成函数注册形式
		//示例：
		//一定要在结束时将m_bRunning设置为false
		//最好提供检测到m_bRunning变为false则停止的功能
		//返回值会被getCurState函数返回给上层调用者
		virtual void mainAlg() {
			fLogCallback(T_TEXT("测试."));
			uint32_t ii = 0;
			while (m_bRunning&&(ii++<m_scheRange)) {
				fScheCallback(ii);
				boost::this_thread::sleep(boost::posix_time::millisec(10));//等待
			}
			if (m_bRunning) {
				m_bRunning = false;
				m_state = 0;
				return;//正常退出
			}
			else {
				m_state = 1;
				return;//非正常退出。m_bRunning是外部改变的
			}
			
		}

	};

}