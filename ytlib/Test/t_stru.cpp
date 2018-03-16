#include "t_stru.h"

namespace ytlib
{
	bool test_heap() {

		
		int data[10] = { 1,2,3,4,5,6,7,8,9,10 };
		Heap<int> h1(data, 10);

		h1.assign(data + 3, 5);

		return true;
	}
}