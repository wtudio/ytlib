/**
 * @file ProcessBase.h
 * @brief 过程基础类
 * @details 定义了基本的过程概念
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once
#include <ytlib/Common/TString.h>
#include <functional>  

namespace ytlib
{
	//日志回调
	typedef std::function<void(const tstring &)> LogCallback;
	//一般过程型程序的类模板。供参考，实际使用时可以参照着写，不一定要继承它
	//非线程安全，不要并发操作
	class ProcessBase
	{
	public:
		ProcessBase():m_bInit(false), m_bRunning(false){
			registerLogCallback(std::bind(&ProcessBase::defLogCallback, this, std::placeholders::_1));
		}
		virtual ~ProcessBase(){
			if (m_bRunning) stop();
		}

		virtual bool init() {
			m_bRunning = false;
			m_bInit = true;
			fLogCallback(T_TEXT("Init Successfully"));
			return true;
		}
		virtual bool start() {
			if (!m_bInit) { 
				fLogCallback(T_TEXT("Start failed : uninitialized"));
				return false; 
			}
			if (m_bRunning) {
				fLogCallback(T_TEXT("Start failed : process is running"));
				return false;
			}
			m_bRunning = true;
			fLogCallback(T_TEXT("Start"));
			return true;
		}
		//pause表示暂停，可以再接着start。stop表示停止，就算重新start也是从头开始
		//此处的pause和stop表现是一样的。交给子类重载
		virtual bool pause() {
			if (!m_bRunning) {
				fLogCallback(T_TEXT("Pause failed : process is not running"));
				return false;
			}
			m_bRunning = false;
			fLogCallback(T_TEXT("Pause"));
			return true;
		}

		virtual bool stop() {
			if (!m_bRunning) {
				fLogCallback(T_TEXT("Stop failed : process is not running"));
				return false;
			}
			m_bRunning = false;
			fLogCallback(T_TEXT("Stop"));
			return true;
		}

		virtual bool isInit() {
			return m_bInit;
		}
		virtual bool isRunning() {
			return m_bRunning;
		}
		//由派生类决定怎样返回当前状态
		virtual int32_t getCurState() {
			return 0;
		}

		inline void registerLogCallback(LogCallback f) {
			fLogCallback = f;
		}

	protected:
		//简单日志功能
		void defLogCallback(const tstring & s) {
			tcout << s << std::endl;
		}
	
		bool m_bInit;
		bool m_bRunning;
		LogCallback fLogCallback;
		
	};


}