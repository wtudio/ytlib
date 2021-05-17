#include "t_Algorithm.h"

#include <ytlib/Common/Util.h>

#include <boost/core/lightweight_test.hpp>
#include <iostream>

using namespace std;
namespace ytlib {
///测试SortAlgs
void test_SortAlgs() {
  YT_DEBUG_PRINTF("test SortAlgs");
  const uint32_t num = 10;
  int answer[num] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  //冒泡
  int data1[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  bubbleSort(data1, num);
  BOOST_TEST_ALL_EQ(data1, data1 + num, answer, answer + num);

  //归并
  int data2[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  mergeSort(data2, num);
  BOOST_TEST_ALL_EQ(data2, data2 + num, answer, answer + num);

  //归并，非递归
  int data2_2[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  mergeSort2(data2_2, num);
  BOOST_TEST_ALL_EQ(data2_2, data2_2 + num, answer, answer + num);

  //快排
  int data3[num] = {1, 4, 2, 8, 5, 9, 0, 7, 6, 3};
  quickSort(data3, num);
  BOOST_TEST_ALL_EQ(data3, data3 + num, answer, answer + num);

  //二分查找
  BOOST_TEST_EQ(binarySearch(answer, num, 6), 6);
  BOOST_TEST_EQ(binarySearch(answer, num, -1), num);

  int data4[num] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 4};
  BOOST_TEST_EQ(binarySearch(data4, num, 2), 4);
  BOOST_TEST_EQ(binarySearch(data4, num, 1), 2);

  int data5[num] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int data6[num] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
  int data7[num] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2};

  BOOST_TEST_EQ(binarySearch(data5, num, 1), num);
  BOOST_TEST_EQ(binarySearch(data6, num, 1), 0);
  BOOST_TEST_EQ(binarySearch(data7, num, 1), num);

  BOOST_TEST_EQ(binarySearchLast(data5, num, 1), num);
  BOOST_TEST_EQ(binarySearchLast(data6, num, 1), 9);
  BOOST_TEST_EQ(binarySearchLast(data7, num, 1), num);
}

}  // namespace ytlib