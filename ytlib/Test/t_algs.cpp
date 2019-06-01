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

	bool test_strFuns() {
		string s = "abc123abc123abc123abc123abc123abc123";

		replaceAll(s, "abc", "yhn");
		printf("%d\t%d\t%X\t%s\n", s.size(), s.capacity(), s.c_str(), s.c_str());

		replaceAll(s, "123", "45");
		printf("%d\t%d\t%X\t%s\n", s.size(), s.capacity(), s.c_str(), s.c_str());

		replaceAll(s, "45", "6789000");
		printf("%d\t%d\t%X\t%s\n", s.size(), s.capacity(), s.c_str(), s.c_str());

		replaceAll(s, "00", "#");
		printf("%d\t%d\t%X\t%s\n", s.size(), s.capacity(), s.c_str(), s.c_str());



		string s1 = "&123%$#456%$#7890#$%";
		vector<string> re = splitAll(s1, "&%$#");
		for (size_t ii = 0; ii < re.size(); ++ii) {
			cout << re[ii] << ' ';
		}
		cout << endl;

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
		//冒泡
		int data1[num] = { 1,4,2,8,5,9,0,7,6,3 };
		bubbleSort(data1, num);
		for (uint32_t ii = 0; ii < num; ++ii) {
			cout << data1[ii] << " ";
		}
		cout << endl;

		//归并
		int data2[num] = { 1,4,2,8,5,9,0,7,6,3 };
		mergeSort(data2, num);
		for (uint32_t ii = 0; ii < num; ++ii) {
			cout << data2[ii] << " ";
		}
		cout << endl;

		//归并，非递归
		int data2_2[num] = { 1,4,2,8,5,9,0,7,6,3 };
		mergeSort2(data2_2, num);
		for (uint32_t ii = 0; ii < num; ++ii) {
			cout << data2_2[ii] << " ";
		}
		cout << endl;


		//快排
		int data3[num] = { 1,4,2,8,5,9,0,7,6,3 };
		quickSort(data3, num);
		for (uint32_t ii = 0; ii < num; ++ii) {
			cout << data3[ii] << " ";
		}
		cout << endl;

		cout << binarySearch(data3, num, 6) << endl;


		int data4[10] = { 0,0,1,1,2,2,2,3,3,4 };

		cout << binarySearch(data4, 10, 2) << endl;
		cout << binarySearch(data4, 10, 1) << endl;

		int data5[10] = { 0,0,0,0,0,0,0,0,0,0 };
		int data6[10] = { 1,1,1,1,1,1,1,1,1,1 };
		int data7[10] = { 2,2,2,2,2,2,2,2,2,2 };

		cout << binarySearch(data5, 10, 1) << endl;
		cout << binarySearch(data6, 10, 1) << endl;
		cout << binarySearch(data7, 10, 1) << endl;

		cout << binarySearchLast(data5, 10, 1) << endl;
		cout << binarySearchLast(data6, 10, 1) << endl;
		cout << binarySearchLast(data7, 10, 1) << endl;

		return true;
	}
}