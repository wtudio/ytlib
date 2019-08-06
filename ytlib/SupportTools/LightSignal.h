/**
 * @file LightSignal.h
 * @brief 轻量级信号量
 * @details 轻量级信号量和相关操作
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once
#include <mutex>
#include <condition_variable>

namespace ytlib
{
	//轻量级信号量
	class LightSignal {
	public:
		LightSignal() :flag(false) {}
		~LightSignal() {}

		void notify() {
			std::lock_guard<std::mutex> lck(m_mutex);
			flag = true;
			m_cond.notify_all();
		}
		void wait() {
			std::unique_lock<std::mutex> lck(m_mutex);
			if (flag) return;
			m_cond.wait(lck);
		}

		bool wait_for(uint32_t timeout_ms) {
			std::unique_lock<std::mutex> lck(m_mutex);
			if (flag) return true;
			if (m_cond.wait_for(lck, std::chrono::milliseconds(timeout_ms)) == std::cv_status::timeout) return false;
			return true;
		}
		void reset() {
			std::lock_guard<std::mutex> lck(m_mutex);
			flag = false;
		}
	private:
		std::mutex m_mutex;
		std::condition_variable m_cond;
		bool flag;
	};
}

  
