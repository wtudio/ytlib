#include "t_tools.h"


namespace ytlib
{
	bool test_SysInfoTools() {
		for (uint32_t ii = 0; ii < 10; ++ii) {
			printf("cpu: %f\tmem:%f", GetCpuUsage(), GetMemUsage());
		}
		return true;
	}
	
}