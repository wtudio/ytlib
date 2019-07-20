#pragma once
#include <ytlib/Common/Util.h>
#include <string>
#include <cstring>
#include <map>
#include <vector>

//字符串相关算法。此处的算法都只是给出一种可行方案，不代表生产中的最佳方案
namespace ytlib {

	//字符串匹配kmp算法。如果没有匹配到则返回sslen。实际使用中应根据情况先做好next字典
	static size_t KMP(const char* ss, size_t sslen, const char* ps, size_t pslen) {
		assert(sslen && pslen && ss!=NULL && ps!= NULL);
		if (pslen > sslen || pslen == 0) return sslen;
		int32_t *next = new int32_t[pslen];//制作next数组
		next[0] = -1;
		int32_t ii = 0, jj = -1;
		--pslen;
		while (ii < pslen) {
			if (jj == -1 || ps[ii] == ps[jj]) next[ii] = (ps[++ii] == ps[++jj]) ? next[jj] : jj;
			else jj = next[jj];
		}
		++pslen;
		ii = jj = 0;
		while (ii < sslen && jj < int32_t(pslen)) {
			if (jj == -1 || ss[ii] == ps[jj]) { ++ii; ++jj; }
			else jj = next[jj];
		}
		delete[] next;
		return (jj == pslen) ? (ii - jj) : sslen;
	}
	static size_t KMP(const std::string& ss, const std::string& ps) {
		return KMP(ss.c_str(), ss.length(), ps.c_str(), ps.length());
	}

	//计算字符串差异度。优化方案只使用2*min(s1len,s2len)的内存而不是s1len*s2len的内存
	static size_t StrDif(const char* s1, size_t s1len, const char* s2, size_t s2len) {
		assert(s1len && s2len && s1 != NULL && s2 != NULL);
		if (s2len > s1len) return StrDif(s2, s2len, s1, s1len);//默认s1len>=s2len

		const uint32_t c1 = 1, c2 = 1;//c1: unmatched cost; c2: mismatched cost
		size_t *M1 = new size_t[s2len], *M2 = new size_t[s2len], *tmpM;
		for (size_t ii = 0; ii < s2len; ++ii) M1[ii] = ii*c1;
		for (size_t ii = 1; ii < s1len; ++ii) {
			for (size_t jj = 0; jj < s2len; ++jj) {
				if (jj == 0) M2[0] = ii*c1;
				else {
					size_t val1 = ((s1[ii] == s2[jj]) ? 0 : c2) + M1[jj - 1];
					size_t val2 = std::min(M1[jj], M2[jj - 1]) + c1;
					M2[jj]= std::min(val1,val2);
				}
			}
			tmpM = M1; M1 = M2; M2 = tmpM;
		}
		size_t re = M1[s2len - 1];
		delete[] M1, M2;
		return re;
	}
	static size_t StrDif(const std::string& s1, const std::string& s2) {
		return StrDif(s1.c_str(), s1.length(), s2.c_str(), s2.length());
	}

	//最长不重复字串。返回其出现的位置和长度
	static std::pair<size_t, size_t> LongestSubStrWithoutDup(const char* s, size_t len) {
		assert(len && s!=NULL);
		size_t positions[256];//每种字符上一次出现的位置
		for (size_t ii = 0; ii < len; ++ii) positions[ii] = len;//初始化为len，表示没出现
		size_t maxLen = 0, maxPos = 0;//最长的字串长度和位置
		size_t curLen = 0, curPos = 0;//当前不重复字串的长度和位置
		for (size_t ii = 0; ii < len; ++ii) {
			size_t &prePos = positions[s[ii]];
			if (prePos == len || (ii - prePos) > curLen) ++curLen;
			else {
				if (curLen > maxLen) { maxLen = curLen;	maxPos = curPos; }
				curLen = ii - prePos;
				curPos = prePos + 1;
			}
			prePos = ii;
		}
		if (curLen > maxLen) { maxLen = curLen;	maxPos = curPos; }
		return std::pair<size_t, size_t>(maxPos, maxLen);
	}
	static std::pair<size_t, size_t> LongestSubStrWithoutDup(const std::string& s) {
		return LongestSubStrWithoutDup(s.c_str(), s.length());
	}


	//替换所有
	static void replaceAll(std::string& str, const std::string& oldValue, const std::string& newValue) {
		std::vector<size_t> vecPos;
		size_t iPos = 0, oldLen = oldValue.size(), newLen = newValue.size();
		while (std::string::npos != (iPos = str.find(oldValue, iPos))) {
			vecPos.push_back(iPos);
			iPos += oldLen;
		}

		size_t& vecLen = iPos = vecPos.size();
		if (vecLen) {
			if (oldLen == newLen) {
				for (size_t ii = 0; ii < vecLen; ++ii)
					memcpy(const_cast<char*>(str.c_str() + vecPos[ii]), newValue.c_str(), newLen);
			}
			else if (oldLen > newLen) {
				char* p = const_cast<char*>(str.c_str()) + vecPos[0];
				vecPos.push_back(str.size());
				for (size_t ii = 0; ii < vecLen; ++ii) {
					memcpy(p, newValue.c_str(), newLen);
					p += newLen;
					size_t cplen = vecPos[ii + 1] - vecPos[ii] - oldLen;
					memmove(p, str.c_str() + vecPos[ii] + oldLen, cplen);
					p += cplen;
				}
				str.resize(p - str.c_str());
			}
			else {
				size_t diff = newLen - oldLen;
				vecPos.push_back(str.size());
				str.resize(str.size() + diff * vecLen);
				char* p = const_cast<char*>(str.c_str()) + str.size();
				for (size_t ii = vecLen - 1; ii < vecLen; --ii) {
					size_t cplen = vecPos[ii + 1] - vecPos[ii] - oldLen;
					p -= cplen;
					memmove(p, str.c_str() + vecPos[ii] + oldLen, cplen);
					p -= newLen;
					memcpy(p, newValue.c_str(), newLen);
				}
			}
		}
	}


	//分割，将str以seperator中所有字符为分割符分割,返回分割结果vector，结果中不包含分隔符
	static std::vector<std::string> splitAll(const std::string& str, const std::string &seperators) {
		std::vector<std::string> re;
		size_t pos1, pos2 = 0;
		do {
			pos1 = str.find_first_not_of(seperators, pos2);
			if (pos1 == std::string::npos) break;
			pos2 = str.find_first_of(seperators, pos1);
			re.push_back(str.substr(pos1, pos2 - pos1));
		} while (pos2 != std::string::npos); 
		return re;
	}
}


