#include "t_algs.h"
#include <iostream>

using namespace std;
namespace ytlib
{
	bool test_kmp() {
		string ss = "bbc abcdab abcdabcdabde";
		string ps = "abcdabd";

		cout << KMP(ss, ps) << endl;

		return true;
	}

	bool test_strdif() {

		string s1 = "abcdxfg";
		string s2 = "abcdefg";

		cout << StrDif(s1, s2) << endl;

		return true;
	}

	bool test_lgsswd() {
		string s = "arabcacfr";

		std::pair<size_t, size_t> re = LongestSubStrWithoutDup(s);
		cout << re.first << "-" << re.second << endl;
		cout << string(s.c_str() + re.first, re.second) << endl;

		return true;
	}

}