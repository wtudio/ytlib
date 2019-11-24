#include "t_Algorithm.h"
#include <iostream>
#include <boost/core/lightweight_test.hpp>
#include <ytlib/Common/Util.h>

using namespace std;
namespace ytlib
{
	///测试SortAlgs
	void test_SortAlgs() {
		YT_DEBUG_PRINTF("test SortAlgs\n");
		const uint32_t num = 10;
		int answer[num] = { 0,1,2,3,4,5,6,7,8,9 };
		//冒泡
		int data1[num] = { 1,4,2,8,5,9,0,7,6,3 };
		bubbleSort(data1, num);
		BOOST_TEST_ALL_EQ(data1, data1 + num, answer, answer + num);
		
		//归并
		int data2[num] = { 1,4,2,8,5,9,0,7,6,3 };
		mergeSort(data2, num);
		BOOST_TEST_ALL_EQ(data2, data2 + num, answer, answer + num);

		//归并，非递归
		int data2_2[num] = { 1,4,2,8,5,9,0,7,6,3 };
		mergeSort2(data2_2, num);
		BOOST_TEST_ALL_EQ(data2_2, data2_2 + num, answer, answer + num);


		//快排
		int data3[num] = { 1,4,2,8,5,9,0,7,6,3 };
		quickSort(data3, num);
		BOOST_TEST_ALL_EQ(data3, data3 + num, answer, answer + num);

		//二分查找
		BOOST_TEST_EQ(binarySearch(answer, num, 6), 6);
		BOOST_TEST_EQ(binarySearch(answer, num, -1), num);

		int data4[num] = { 0,0,1,1,2,2,2,3,3,4 };
		BOOST_TEST_EQ(binarySearch(data4, num, 2), 4);
		BOOST_TEST_EQ(binarySearch(data4, num, 1), 2);

		int data5[num] = { 0,0,0,0,0,0,0,0,0,0 };
		int data6[num] = { 1,1,1,1,1,1,1,1,1,1 };
		int data7[num] = { 2,2,2,2,2,2,2,2,2,2 };

		BOOST_TEST_EQ(binarySearch(data5, num, 1), num);
		BOOST_TEST_EQ(binarySearch(data6, num, 1), 0);
		BOOST_TEST_EQ(binarySearch(data7, num, 1), num);

		BOOST_TEST_EQ(binarySearchLast(data5, num, 1), num);
		BOOST_TEST_EQ(binarySearchLast(data6, num, 1), 9);
		BOOST_TEST_EQ(binarySearchLast(data7, num, 1), num);

	}

	///测试StringAlgs
	void test_StringAlgs() {
		YT_DEBUG_PRINTF("test StringAlgs\n");
		//kmp
		string ss = "abcdef abcdefg abcdefgh";
		BOOST_TEST_EQ(KMP(ss, "abcdef"), 0);
		BOOST_TEST_EQ(KMP(ss, "abcdefg"), 7);
		BOOST_TEST_EQ(KMP(ss, "abcdefgh"), 15);
		BOOST_TEST_EQ(KMP(ss, "aaaa"), ss.length());

		//StrDif
		BOOST_TEST_EQ(StrDif("abcdxfg", "abcdefg"), 1);
		BOOST_TEST_EQ(StrDif("abcdfg", "abcdefg"), 1);
		BOOST_TEST_EQ(StrDif("abcdfg", "abcdef"), 2);


		//LongestSubStrWithoutDup
		pair<size_t, size_t> re = LongestSubStrWithoutDup("arabcacfr");
		BOOST_TEST_EQ(re.first, 1);
		BOOST_TEST_EQ(re.second, 4);

		//replaceAll
		string s = "abc123abc123abc123abc123abc123abc12";
		int len = s.size();
		const char* p = s.c_str();
		replaceAll(s, "abc", "yhn");
		BOOST_TEST_CSTR_EQ(s.c_str(), "yhn123yhn123yhn123yhn123yhn123yhn12");
		BOOST_TEST_EQ(s.size(), len);
		BOOST_TEST_EQ(s.c_str(), p);


		replaceAll(s, "123", "45");
		BOOST_TEST_CSTR_EQ(s.c_str(), "yhn45yhn45yhn45yhn45yhn45yhn12");
		BOOST_TEST_LT(s.size(), len);
		BOOST_TEST_EQ(s.c_str(), p);

		replaceAll(s, "45", "6789000");
		BOOST_TEST_CSTR_EQ(s.c_str(), "yhn6789000yhn6789000yhn6789000yhn6789000yhn6789000yhn12");
		BOOST_TEST_GT(s.size(), len);
		BOOST_TEST_NE(s.c_str(), p);

		replaceAll(s, "00", "#");
		BOOST_TEST_CSTR_EQ(s.c_str(), "yhn6789#0yhn6789#0yhn6789#0yhn6789#0yhn6789#0yhn12");

		//splitAll
		vector<string> re2 = splitAll("&123%$#456%$#7890#$%", "&%$#");
		vector<string> answer2{ "123","456","7890" };
		BOOST_TEST_ALL_EQ(re2.begin(), re2.end(), answer2.begin(), answer2.end());

	}

	
}