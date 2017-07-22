#pragma once

#include <queue>
#include <atomic>
#include <mutex>

namespace ytlib {

	//简单的多线程环境下线程安全的队列
	template <class T>
	class QueueBase{
	public:
		explicit QueueBase(size_t n_):m_maxcount(n_){}
		virtual ~QueueBase(){}
		inline size_t GetMaxCount(){ return m_maxcount; }
		inline size_t Count() {
			std::lock_guard<std::mutex> lck(m_mutex);
			return m_queue.size();
		}
		inline void Clear() {
			std::lock_guard<std::mutex> lck(m_mutex);
			m_queue.swap(std::queue<T>());
		}
		//添加元素
		bool Enqueue(const T &item) {
			std::lock_guard<std::mutex> lck(m_mutex);
			if (m_queue.size() < m_maxcount) {
				m_queue.push(std::move(item));
				m_cond.notify_one();
				return true;
			}
			return false;
		}
		//取出元素
		bool Dequeue(T &item) {
			std::lock_guard<std::mutex> lck(m_mutex);
			if (m_queue.empty()) return false; 
			item = std::move(m_queue.front());
			m_queue.pop();
			return true;

		}
		
		//阻塞式取出。如果空了就一直等待到可以取出。（没有做阻塞式添加，因为一般不可能用到）
		bool BlockDequeue(T &item) {
			std::unique_lock<std::mutex> lck(m_mutex);
			m_cond.wait(lck, [this] {return !(this->m_queue.empty()); });
			item = std::move(m_queue.front());
			m_queue.pop();
			return true;
		}

	protected:
		std::mutex m_mutex;//同步锁。此处不能用读写锁，因为condition_variable还不支持
		std::condition_variable m_cond;//条件锁
		std::queue<T> m_queue;// 队列
		
		const size_t m_maxcount;//队列可支持最大个数
	};
}