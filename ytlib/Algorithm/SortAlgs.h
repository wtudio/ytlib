#pragma once
#include <ytlib/Common/Util.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

/*
stl中有很多成熟的算法可以直接调用（stl源码剖析p288）
此处自己实现的排序算法供学习与改造
todo：待完善
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
		bool operator <(const sortObj& val) {return key < val.key;}
		bool operator >(const sortObj& val) {return key > val.key;}
		bool operator <=(const sortObj& val) {return key <= val.key;}
		bool operator >=(const sortObj& val) {return key >= val.key;}
		bool operator ==(const sortObj& val) {return key == val.key;}


		//成员
		uint32_t key;
		//成员如果较大的话用指针指着

	};



	//冒泡排序 in-place/稳定
	template<typename T>
	void bubbleSort(T* arr, size_t len) {
		using std::swap;
		for (size_t ii = 0; ii < len; ++ii) {
			for (size_t jj = 0; jj < len - 1 - ii; ++jj) {
				if (arr[jj] > arr[jj + 1]) {
					swap(arr[jj], arr[jj + 1]);
				}
			}
		}
	}



	//归并排序 out-place/稳定。todo：还没完成
	template<typename T>
	void mergeSort(T* arr, size_t len) {
		if (len < 2) return;
		size_t llen = len / 2, rlen = len - llen;
		
		mergeSort(arr, llen);
		mergeSort(arr + llen, rlen);
		//在另外的空间排序好了再复制回来
		T* tmpArr = new T[len];
		size_t lc = 0, rc = llen, tc = 0;
		while ((lc<llen) && (rc<len)) {
			if (arr[lc] < arr[rc]) {
				tmpArr[tc++] = arr[lc++];
			}
			else {
				tmpArr[tc++] = arr[rc++];
			}
		}

		if (lc < llen && rc==len) {
			memcpy(arr + lc + llen, arr + lc, (llen - lc) * sizeof(T));
		}
		memcpy(arr, tmpArr, tc * sizeof(T));
		delete[] tmpArr;
	}

	//快速排序 in-place/不稳定
	template<typename T>
	void quickSort(T* arr, size_t len) {
		using std::swap;


	}



	//
	


	//二分查找



}


