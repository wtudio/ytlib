#pragma once
#include <ytlib/Common/Util.h>
#include <cmath>

#define USE_DOUBLE_PRECISION

#if defined(USE_DOUBLE_PRECISION)
typedef double tfloat;      // double precision
#else 
typedef float  tfloat;    // single precision
#endif 

#define PI 3.1415926535897932384626433832795028841971

//判断浮点数是否相等
#ifndef fequal
#define fequal(a,b)          (((a-b)>-1e-6)&&((a-b)<1e-6))
#endif

//一些工具
namespace ytlib
{
	//计算有多少个1
	static uint32_t count_1_(uint32_t n) {
		uint32_t num = 0;
		while (n) {
			n &= (n - 1);
			++num;
		}
		return num;
	}

	//计算有多少个0
	static uint32_t count_0_(uint32_t n) {
		return count_1_(~n);
	}

	//判断是否是质数
	static bool isPrime(uint32_t num) {
		//两个较小数另外处理  
		if (num == 2 || num == 3)
			return true;
		//不在6的倍数两侧的一定不是质数  
		if (num % 6 != 1 && num % 6 != 5)
			return false;
		int tmp = std::sqrt(num);
		//在6的倍数两侧的也可能不是质数  
		for (int i = 5; i <= tmp; i += 6)
			if (num %i == 0 || num % (i + 2) == 0)
				return false;
		//排除所有，剩余的是质数  
		return true;

	}
	//求最大公约数。最小公倍数：num1*num2/gcd(num1,num2)
	static uint32_t gcd(uint32_t num1, uint32_t num2) {
		if (num1 < num2) return gcd(num2, num1);
		if (num2 == 0) return num1;
		if (num1 & 1) {
			if (num2 & 1) return gcd(num2, num1 - num2);
			return gcd(num1, num2 >> 1);

		}
		else {
			if (num2 & 1) return gcd(num1 >> 1, num2);
			return (gcd(num1 >> 1, num2 >> 1) << 1);
		}
	}




}