#include "t_tools.h"
#include <ytlib/SupportTools/UUID.h>

namespace ytlib
{
	bool test_SysInfoTools() {
		for (uint32_t ii = 0; ii < 10; ++ii) {
			printf("cpu: %f\tmem:%f\n", GetCpuUsage(), GetMemUsage());
		}
		return true;
	}


	class test_a {
		T_CLASS_SERIALIZE(&s&a&ps&ps2)
	public:

		std::string s;
		uint32_t a;
		boost::shared_ptr<test_a> ps;
		boost::shared_ptr<test_a> ps2;
		
	};


	bool test_Serialize() {
		test_a obj1;
		obj1.s = "dddd";
		obj1.a = 100;
		std::string re;
		Serialize(obj1, re, SerializeType::BinaryType);

		test_a obj2;
		Deserialize(obj2, re, SerializeType::BinaryType);

		return true;
	}
}