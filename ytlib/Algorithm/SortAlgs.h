#pragma once
#include <ytlib/Common/Util.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

/*
stl中有很多成熟的算法可以直接调用（stl源码剖析p288）
此处自己实现的排序算法供学习与改造
所有排序都是从小到大排列
*/

//模板化的排序相关算法
namespace ytlib {

	//重载了比较符号的类
	class sortObj {
	public:
		sortObj():key(0){}
		virtual ~sortObj() {}

		sortObj(uint32_t k_) :key(k_) {}

		//关系运算符重载
		bool operator <(const sortObj& val) const {return key < val.key;}
		bool operator >(const sortObj& val) const {return key > val.key;}
		bool operator <=(const sortObj& val) const {return key <= val.key;}
		bool operator >=(const sortObj& val) const {return key >= val.key;}
		bool operator ==(const sortObj& val) const {return key == val.key;}


		//成员
		uint32_t key;
		//成员如果较大的话用指针指着

	};

	//冒泡排序 in-place/稳定
	template<typename T>
	void bubbleSort(T* arr, size_t len) {
		if (len < 2) return;
		using std::swap;
		for (size_t ii = 0; ii < len; ++ii) {
			for (size_t jj = 0; jj < len - 1 - ii; ++jj) {
				if (arr[jj] > arr[jj + 1]) {
					swap(arr[jj], arr[jj + 1]);
				}
			}
		}
	}

	//归并排序 out-place/稳定
	template<typename T>
	void mergeSort(T* arr, size_t len) {
		if (len < 2) return;
		if (len == 2) {
			using std::swap;
			if (arr[0] > arr[1]) swap(arr[0], arr[1]);
			return;
		}
		size_t middle = len / 2;
		mergeSort(arr, middle);
		mergeSort(arr + middle, len - middle);
		//在另外的空间排序好了再复制回来
		T* tmpArr = new T[len];
		size_t lc = 0, rc = middle, tc = 0;
		while ((lc<middle) && (rc<len)) {
			tmpArr[tc++] = (arr[lc] < arr[rc]) ? arr[lc++] : arr[rc++];
		}
		if (rc == len) memcpy(arr + tc, arr + lc, (middle - lc) * sizeof(T));
		memcpy(arr, tmpArr, tc * sizeof(T));
		delete[] tmpArr;
	}

	//归并排序 out-place/稳定，非递归形式，空间复杂度O(n)
	template<typename T>
	void mergeSort2(T* arr, size_t len) {
		if (len < 2) return;
		using std::swap;
		if (len == 2) {
			if (arr[0] > arr[1]) swap(arr[0], arr[1]);
			return;
		}

		T *tmpArr1 = new T[len], *tmpArr0 = arr;
		for (size_t ii = 1; ii < len; ii <<= 1) {
			size_t jj = 0;
			while (jj + ii < len) {
				size_t lc = jj, rc = jj + ii, tc = jj, middle = jj + ii, last = jj + 2 * ii;
				if (last > len) last = len;

				while ((lc < middle) && (rc < last)) {
					tmpArr1[tc++] = (tmpArr0[lc] < tmpArr0[rc]) ? tmpArr0[lc++] : tmpArr0[rc++];
				}
				if (rc == last) memcpy(tmpArr1 + tc, tmpArr0 + lc, (middle - lc) * sizeof(T));
				else if (lc == middle) memcpy(tmpArr1 + tc, tmpArr0 + rc, (last - rc) * sizeof(T));

				if (last == len) {
					jj = len;
					break;
				}
				jj += 2 * ii;
			}
			if (jj < len) {
				memcpy(tmpArr1 + jj, tmpArr0 + jj, (len - jj) * sizeof(T));
			}
			swap(tmpArr1, tmpArr0);
		}
		if (tmpArr0 != arr) {
			memcpy(arr, tmpArr0, len * sizeof(T));
			delete[] tmpArr0;
		}
		else {
			delete[] tmpArr1;
		}

	}

	//快速排序 in-place/不稳定
	template<typename T>
	void quickSort(T* arr, size_t len) {
		if (len < 2) return;
		using std::swap;
		if (len == 2) {
			if (arr[0] > arr[1]) swap(arr[0], arr[1]);
			return;
		}
		size_t first = 0, last = len - 1, cur = first;
		while (first < last) {
			while (first < last && arr[last] >= arr[cur]) --last;
			if (cur != last) {
				swap(arr[cur], arr[last]);
				cur = last;
			}
			while (first < last && arr[first] <= arr[cur]) ++first;
			if (cur != first) {
				swap(arr[cur], arr[first]);
				cur = first;
			}
		}
		quickSort(arr, cur);
		quickSort(arr + cur + 1, len - cur - 1);
	}

	//二分查找。应用于从小到大排序好的数组中，如有重复则找首先出现的那个
	template<typename T>
	size_t binarySearch(T* arr, size_t len,const T& key) {
		assert(len);
		size_t low = 0, high = len - 1;
		while (low < high) {
			size_t mid = low + (high - low) / 2;
			if(arr[mid] < key) low = mid + 1; 
			else high = mid;
		}
		if (arr[low] == key) return low;
		//没找到，返回len
		return len;
	}

	//二分查找。应用于从小到大排序好的数组中，如有重复则找最后出现的那个
	template<typename T>
	size_t binarySearchLast(T* arr, size_t len, const T& key) {
		assert(len);
		size_t low = 0, high = len - 1;
		while (low < high) {
			size_t mid = low + (high - low + 1) / 2;
			if (arr[mid] <= key) low = mid;
			else high = mid - 1;
		}
		if (arr[high] == key) return high;
		//没找到，返回len
		return len;
	}

}


