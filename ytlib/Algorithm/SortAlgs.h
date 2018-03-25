#pragma once
#include <ytlib/Common/Util.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

/*
stl中有很多成熟的算法可以直接调用（stl源码剖析p288）
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


	};


	

}


