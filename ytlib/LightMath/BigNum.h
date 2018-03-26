#pragma once
#include <iostream>
#include <ytlib/LightMath/mathbase.h>
#include <vector>

//大整数数工具
namespace ytlib
{
	class BigNum {
	public:
		BigNum(bool symbol = true) :_symbol(symbol) {}
		virtual ~BigNum(){}

		BigNum(const std::vector<uint32_t>& data, bool symbol = true) :_symbol(symbol), _content(data) {

		}
		BigNum(const uint32_t* a, size_t sz, bool symbol = true) :_symbol(symbol) {
			_content.reserve(sz);
			_content.assign(a, a + sz);
		}

		BigNum  operator+ (const BigNum &value) {
			
		}
		BigNum&  operator+= (const BigNum &value) {
			
			return *this;
		}
		BigNum& operator++() {

		}



		BigNum  operator- (const BigNum &value) {
			
		}
		BigNum&  operator-= (const BigNum &value) {
			
			return *this;
		}

		BigNum  operator* (const BigNum &value) {
			
		}
		BigNum&  operator*= (const BigNum &value) {
			
			return *this;
		}
		BigNum  operator/ (const BigNum &value) {
			
		}
		BigNum&  operator/= (const BigNum &value) {
			
			return *this;
		}



	protected:
		//储存
		bool _symbol;//正负
		std::vector<uint32_t> _content;
	};
	
	/*
	基于大数工具的循环辅助工具。可以实现n层循环
	使用时：
	LoopTool lt;
	do{

	}while(--lt);
	*/
	class LoopTool :public BigNum {
	public:



	};
	
}

