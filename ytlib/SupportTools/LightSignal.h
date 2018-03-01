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

  
