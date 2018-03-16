#pragma once
#include <vector>

namespace ytlib {

#define LEFT_CHILD(x)	(x << 1 + 1)
#define RIGHT_CHILD(x)	(x << 1 + 2)
#define PARENT(x)		((x-1)>>1)


	//堆
	template<typename T>
	class Heap {
	public:
		Heap(bool _type = true) : type(_type) {}
		virtual ~Heap() {}

		Heap(const std::vector<T>& a, bool _type = true) :_a(a), type(_type) {
			sort();
		}
		Heap(const T* a, size_t sz, bool _type = true) :type(_type) {
			_a.reserve(sz);
			_a.assign(a, a + sz);
			sort();
		}

		void assign(const T* a, size_t sz) {
			_a.clear();
			_a.reserve(sz);
			_a.assign(a, a + sz);
			sort();
		}

		void push(const T& val) {

		}


		void pop() {

		}

		void sort() {
			for (size_t ii = (_a.size() - 2) >> 1; ii > 0;; --ii) {
				adjustDown(ii);
			}
			adjustDown(0);
		}

		void adjustDown(size_t index) {
			size_t child = index << 1 + 1;
			size_t len = _a.size();
			while (child < len) {
				if (((child + 1) < len) && (_a[child + 1] > _a[child])) {
					++child;
				}
				if (_a[child] > _a[index]) {
					swap(_a[child], _a[index]);//T需要实现swap方法
					index = child;
					child = index << 1 + 1;
				}
				else break;
			}

		}

		void adjustUp(size_t index) {

		}

		const bool type;//true为最大堆，否则最小堆
		//可以直接公开访问
		std::vector<T> _a;
		
	};


}


