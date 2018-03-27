#pragma once
#include <iostream>
#include <ytlib/LightMath/mathbase.h>
#include <vector>

//大整数数工具。todo待完善
namespace ytlib
{
	class BigNum {
	public:
		explicit BigNum(bool symbol = true, uint32_t _up = 10) :_symbol(symbol), up(_up) {
			assert(up > 1);//至少二进制
			_content.push_back(0);
		}
		virtual ~BigNum() {}


		explicit BigNum(const std::vector<uint32_t>& data, bool symbol = true, uint32_t _up = 10) :_symbol(symbol), _content(data), up(_up) {

		}
		BigNum(const uint32_t* a, size_t sz, bool symbol = true, uint32_t _up = 10) :_symbol(symbol), up(_up) {
			_content.reserve(sz);
			_content.assign(a, a + sz);
		}
		//这里允许将一个int64隐式转换到BigNum
		BigNum(int64_t num, uint32_t _up = 10) :_symbol(num >= 0), up(_up) {

		}


		BigNum  operator+ (const BigNum &value) {
			
		}
		BigNum&  operator+= (const BigNum &value) {
			
			return *this;
		}
		BigNum& operator++() {

			return *this;
		}
		const BigNum operator++(int) {

			
		}


		BigNum  operator- (const BigNum &value) {
			
		}
		BigNum&  operator-= (const BigNum &value) {
			
			return *this;
		}

		BigNum& operator--() {

			return *this;
		}
		const BigNum operator--(int) {


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

		bool operator==(const BigNum &value) {

		}
		//是否为0
		operator bool() {
			return ((_content.size() == 1) && (_content[0] == 0));
		}
		BigNum  operator- () {

		}

		//以十进制形式输入
		friend std::istream& operator>>(std::istream& in, const BigNum& M) {

			return in;
		}

		friend std::ostream& operator<< (std::ostream& out, const BigNum& M) {
			//先输出进制

			//符号位
			if (!_symbol && BigNum::operator bool()) out << '-';

			
			if (up <= 16) {
				//如果进制在16之内则采用16进制的符号


			}
			else {
				//否则需要在各个位之间空开一格，以十进制输出数据


			}
			return out;
		}

	protected:
		//储存
		bool _symbol;//正负
		std::vector<uint32_t> _content;//采用大端存储，越高位在越后面，方便增加位数

		uint32_t up;//进制
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
		LoopTool(){}
		virtual ~LoopTool() {}



	private:
		//隐藏一些函数，只支持+、-
		friend std::istream& operator>>(std::istream& in, const LoopTool& M) {return in;}
		friend std::ostream& operator<< (std::ostream& out, const LoopTool& M) {return out;}




		std::vector<uint32_t> up;//进制

	};
	
}

