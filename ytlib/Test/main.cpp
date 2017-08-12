#include <ytlib/Common/Util.h>

#include "t_lmath.h"
#include "t_file.h"
#include "t_process.h"
#include "t_net.h"
#include "t_log.h"
#include "t_tools.h"

#include <boost/date_time/posix_time/posix_time.hpp>  

using namespace std;
using namespace ytlib;

uint64_t tnow;
void setTime() {
	tnow = static_cast<uint64_t>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}
std::string getTime() {
	std::time_t t_time(static_cast<std::time_t>(tnow));
	return std::string(std::ctime(&t_time));
}
int32_t main(int32_t argc, char** argv) {
	YT_DEBUG_PRINTF("-------------------start-------------------\n");
	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));


	//boost::posix_time::ptime ticTime_global, tocTime_global;
	//ticTime_global = boost::posix_time::microsec_clock::universal_time();

	//tocTime_global = boost::posix_time::microsec_clock::universal_time(); 
	//std::cout << (tocTime_global - ticTime_global).ticks() << "us" << std::endl;
	
	//setTime();
	//cout << getTime() << endl;

	test_Serialize();
	test_SysInfoTools();
	test_NetLog();
	test_TcpNetAdapter();
	

	test_Complex();
	test_Matrix();
	test_Matrix_c();

	test_KeyValueFile();
	test_SerializeFile();
	test_XMLFile();
	test_PrjBase();

	test_QueueProcess();
	
	printf("******************end*******************\n");
	getchar();
	
}
