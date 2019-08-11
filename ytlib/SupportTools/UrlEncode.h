/**
 * @file UrlEncode.h
 * @brief Url编解码
 * @details UrlEncode、UrlDecode
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#pragma once

#include <ytlib/Common/Util.h>

namespace ytlib
{
	inline unsigned char ToHex(unsigned char x, bool up) {
		return  x > 9 ? x + (up ? 55 : 87) : x + 48;
	}

	inline unsigned char FromHex(unsigned char x) {
		unsigned char y;
		if (x >= 'A' && x <= 'Z') y = x - 55;
		else if (x >= 'a' && x <= 'z') y = x - 87;
		else if (x >= '0' && x <= '9') y = x - '0';
		else assert(0);
		return y;
	}
	/**
	 * @brief UrlEncode
	 * @param str 待编码字符串
	 * @param up 是否转码为大写字符
	 * @return 转码后的结果字符串
	 */
	static std::string UrlEncode(const std::string& str, bool up = true) {
		std::string strTemp;
		size_t length = str.length();
		strTemp.reserve(length << 1);
		for (size_t i = 0; i < length; ++i) {
			if (isalnum((unsigned char)str[i]) || (str[i] == '-') || (str[i] == '_') || (str[i] == '.') || (str[i] == '~'))
				strTemp += str[i];
			else if (str[i] == ' ')
				strTemp += '+';
			else {
				strTemp += '%';
				strTemp += ToHex((unsigned char)str[i] >> 4, up);
				strTemp += ToHex((unsigned char)str[i] & 15, up);
			}
		}
		return strTemp;
	}
	/**
	 * @brief UrlDecode
	 * @param str 待解码字符串
	 * @return 解码后的结果字符串
	 */
	static std::string UrlDecode(const std::string& str) {
		std::string strTemp;
		size_t length = str.length();
		strTemp.reserve(length);
		for (size_t i = 0; i < length; ++i) {
			if (str[i] == '+') strTemp += ' ';
			else if (str[i] == '%') {
				assert(i + 2 < length);
				unsigned char c = (FromHex((unsigned char)str[++i])) << 4;
				strTemp += (c | FromHex((unsigned char)str[++i]));
			}
			else strTemp += str[i];
		}
		return strTemp;
	}
	
}

  
