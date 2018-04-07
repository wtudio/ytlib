#pragma once
#include <iostream>
#include <ytlib/LightMath/Mathbase.h>
#include <vector>

//大整数工具。todo待完善
namespace ytlib
{
	class BigNum {
	public:
		explicit BigNum(bool symbol = true, uint32_t _up = 10) :_symbol(symbol), up(_up) {
			assert(up != 1);
			_content.push_back(0);
		}
		//这里允许将一个int64隐式转换到BigNum
		BigNum(int64_t num, uint32_t _up = 0) :_symbol(num >= 0), up(0) {
			num = std::abs(num);
			_content.push_back(static_cast<uint32_t>(num));
			_content.push_back(static_cast<uint32_t>(num>>32));
			changeNumSys(_up);
		}

		BigNum  operator+ (const BigNum &value) const {
			//需要确保进制相同
			assert(up == value.up);
			//被加数的位数较大
			if (_content.size() < value._content.size()) return value + (*this);
			size_t len1 = value._content.size(), len2 = _content.size();
			BigNum re(true, up);
			if (_symbol^value._symbol) {
				//异符号相加，用大的减小的

			}
			else {
				//同符号相加
				re._symbol = _symbol;
				//从低位开始加
				for (size_t ii = 0; ii < len1; ++ii) {
					uint32_t tmp = _content[ii] + value._content[ii] + re._content[ii];
					if (tmp >= up || tmp < _content[ii] || (tmp == _content[ii] && re._content[ii] == 1)) {
						re._content.push_back(1);
						tmp -= up;
					}
					else re._content.push_back(0);
					re._content[ii] = tmp;
				}
				for (size_t ii = len1; ii < len2; ++ii) {
					if ( _content[ii]==(up-1) && re._content[ii] == 1) {
						re._content[ii] = 0;
						re._content.push_back(1);
					}
					else {
						re._content[ii] += _content[ii];
						re._content.push_back(0);
					}
				}
			}
			//去除最后端的0
			if (re._content.size() > 1 && re._content[re._content.size() - 1] == 0) re._content.pop_back();

		}
		BigNum&  operator+= (const BigNum &value) {
			assert(up == value.up);


			return *this;
		}
		//++i
		BigNum& operator++() {

			return *this;
		}
		//i++
		const BigNum operator++(int) {

			
		}


		BigNum  operator- (const BigNum &value) const {
			assert(up == value.up);

		}
		BigNum&  operator-= (const BigNum &value) {
			assert(up == value.up);

			return *this;
		}

		BigNum& operator--() {

			return *this;
		}
		const BigNum operator--(int) {


		}

		BigNum  operator* (const BigNum &value) const {
			assert(up == value.up);

		}
		BigNum&  operator*= (const BigNum &value) {
			assert(up == value.up);

			return *this;
		}
		BigNum  operator/ (const BigNum &value) const {
			assert(up == value.up);

		}
		BigNum&  operator/= (const BigNum &value) {
			assert(up == value.up);

			return *this;
		}

		bool operator==(const BigNum &value) const {
			assert(up == value.up);
			//如果都是0
			if (BigNum::operator bool() && value) return true;
			if (_symbol != value._symbol) return false;
			if (_content.size() != value._content.size()) return false;
			size_t len = _content.size();
			for (size_t ii = 0; ii < len; ++ii) {
				if (_content[ii] != value._content[ii]) return false;
			}
			return true;
		}
		//是否为0。0为false
		operator bool() const {
			return !((_content.size() == 1) && (_content[0] == 0));
		}
		BigNum  operator- () const {
			BigNum re(*this);
			re._symbol = !re._symbol;
			return re;
		}
		bool operator <(const BigNum& value) const {
			assert(up == value.up);
			if (BigNum::operator bool() && value) return false;
			if (!_symbol && value._symbol) return true;
			if (_symbol && !(value._symbol)) return false;
			if (_content.size() != value._content.size()) return _symbol ^ (_content.size() > value._content.size());
			size_t len = _content.size();
			//从高位开始判断
			for (size_t ii = len - 1; ii > 0; --ii) {
				if (_symbol ^ (_content[ii] >= value._content[ii])) return true;
			}
			return _symbol ^ (_content[0] >= value._content[0]);
		}
		bool operator >(const BigNum& value) const {
			return value < (*this);
		}
		bool operator <=(const BigNum& value) const {
			return BigNum::operator<(value) || BigNum::operator==(value);
		}
		bool operator >=(const BigNum& value) const {
			return BigNum::operator>(value) || BigNum::operator==(value);
		}

		//以十进制形式输入
		friend std::istream& operator>>(std::istream& in, const BigNum& M) {

			return in;
		}

		friend std::ostream& operator<< (std::ostream& out, const BigNum& M) {
			//先输出进制

			//符号位
			if (!_symbol && !(BigNum::operator bool())) out << '-';

			
			if (up <= 16) {
				//如果进制在16之内则采用16进制的符号


			}
			else {
				//否则需要在各个位之间空开一格，以十进制输出数据


			}
			return out;
		}

		//改变进制
		void changeNumSys(uint32_t up_) {
			assert(up_ != 1);
			if (up_ == up) return;


		}

	protected:

		bool _symbol;//正负
		std::vector<uint32_t> _content;//采用大端存储，越高位在越后面，方便增加位数
		uint32_t up;//进制，不能等于1。0表示就是以2^32为进制
	};
	
	/*
	基于大数工具的循环辅助工具。可以实现n层循环。虽然一般不要出现n层循环
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

