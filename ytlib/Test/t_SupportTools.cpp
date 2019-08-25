#include "t_SupportTools.h"
#include <ytlib/SupportTools/UUID.h>
#include <ytlib/SupportTools/UrlEncode.h>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <numeric>
using namespace std;
namespace ytlib
{

	class test_a {
		T_CLASS_SERIALIZE(&s&a&ps&ps2)
	public:

		string s;
		uint32_t a;
		boost::shared_ptr<test_a> ps;
		boost::shared_ptr<test_a> ps2;
		
	};


	void test_Serialize() {
		test_a obj1;
		obj1.s = "dddd";
		obj1.a = 100;
		string re;
		Serialize(obj1, re, SerializeType::BinaryType);

		test_a obj2;
		Deserialize(obj2, re, SerializeType::BinaryType);

		
	}

	//这个函数只是用于学习stl库里的算法
	void test_stl() {

		int re = 0;
		vector<int> v1{ 5,4,8,2,1,3,9,7,6 };//未排序数组1
		
		re = accumulate(v1.begin(), v1.end(), 0);
		cout << re << endl;




		
	}

	void test_urlencode() {
		string s2 = "http://abc123.com/aaa/bbbb?qa=1&qb=adf";
		printf("%d\t%d\t%X\t%s\n", s2.size(), s2.capacity(), s2.c_str(), s2.c_str());

		string s3 = UrlEncode(s2, false);
		printf("%d\t%d\t%X\t%s\n", s3.size(), s3.capacity(), s3.c_str(), s3.c_str());

		string s4 = UrlDecode(s3);
		printf("%d\t%d\t%X\t%s\n", s4.size(), s4.capacity(), s4.c_str(), s4.c_str());


		
	}

	void test_LoopTool() {
		LoopTool lt(vector<uint32_t>{2, 3, 3});
		do {
			for (uint32_t ii = lt.m_vecContent.size() - 1; ii > 0; --ii) {
				cout << lt.m_vecContent[ii] << '-';
			}
			cout << lt.m_vecContent[0] << endl;

		} while (++lt);
		

	}

}