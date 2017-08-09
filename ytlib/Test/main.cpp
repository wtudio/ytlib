#include <ytlib/Common/Util.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/SupportTools/UUID.h>
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <ytlib/LogService/LoggerServer.h>
#include <ytlib/LogService/Logger.h>
#include <ytlib/SupportTools/SysInfoTools.h>

#include "mathtest.h"
#include "filetest.h"
#include "processtest.h"
#include "nettest.h"

#include <boost/date_time/posix_time/posix_time.hpp>  

using namespace std;
using namespace ytlib;


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
	
	cout << GetCpuUsage() << endl;

	LoggerServer l(55555);
	l.start();

	InitNetLog(12345, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 55555));

	YT_LOG_TRACE << "trace log test";
	YT_LOG_DEBUG << "debug log test";
	YT_LOG_INFO << "info log test";
	YT_LOG_WARNING << "warning log test";

	YT_SET_LOG_LEVEL(debug);

	YT_LOG_TRACE << "trace log test";
	YT_LOG_DEBUG << "debug log test";
	YT_LOG_INFO << "info log test";
	YT_LOG_WARNING << "warning log test";

	getchar();

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
