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

	bool test_sort() {
		const uint32_t num = 10;
		//Ã°ÅÝ
		int data1[num] = { 1,4,2,8,5,9,0,7,6,3 };
		bubbleSort(data1, num);
		for (uint32_t ii = 0; ii < num; ++ii) {
			cout << data1[ii] << " ";
		}
		cout << endl;

		//¹é²¢
		int data2[num] = { 1,4,2,8,5,9,0,7,6,3 };
		mergeSort(data2, num);
		for (uint32_t ii = 0; ii < num; ++ii) {
			cout << data2[ii] << " ";
		}
		cout << endl;





		return true;
	}
}