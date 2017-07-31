#include <ytlib/Common/Util.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/SupportTools/UUID.h>
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <ytlib/LogService/LoggerServer.h>

#include "mathtest.h"
#include "filetest.h"
#include "processtest.h"
#include "nettest.h"

#include <boost/date_time/posix_time/posix_time.hpp>  
#include <sigar/include/sigar.h>
extern "C"
{
#include  <sigar/include/sigar_format.h>
}



using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
	printf_s("-------------------start-------------------\n");
	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));


	//boost::posix_time::ptime ticTime_global, tocTime_global;
	//ticTime_global = boost::posix_time::microsec_clock::universal_time();

	//tocTime_global = boost::posix_time::microsec_clock::universal_time(); 
	//std::cout << (tocTime_global - ticTime_global).ticks() << "us" << std::endl;

	sigar_t *sigar_cpu;
	sigar_cpu_t old;
	sigar_cpu_t current;
	sigar_cpu_perc_t perc;

	sigar_t *sigar_mem;
	sigar_mem_t currentmem;

	sigar_open(&sigar_cpu);
	sigar_cpu_get(sigar_cpu, &old);

	sigar_open(&sigar_mem);

	

	for (int ii = 0; ii < 10000; ++ii) {

		sigar_cpu_get(sigar_cpu, &current);
		sigar_cpu_perc_calculate(&old, &current, &perc);

		sigar_mem_get(sigar_mem, &currentmem);
		old = current;

		printf_s("cpu : %f \t mem : %f\n", perc.combined, (double)currentmem.used / (double)currentmem.total);
		Sleep(100);
	}


	getchar();

	test_TcpNetAdapter();
	
	LoggerServer l(55555);
	l.start();


	test_Complex();
	test_Matrix();
	test_Matrix_c();

	test_KeyValueFile();
	test_SerializeFile();
	test_XMLFile();
	test_PrjBase();

	test_QueueProcess();
	
	printf_s("******************end*******************\n");
	getchar();
	
}
