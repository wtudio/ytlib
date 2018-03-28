#include <ytlib/Common/Util.h>

#include "t_lmath.h"
#include "t_file.h"
#include "t_process.h"
#include "t_net.h"
#include "t_log.h"
#include "t_tools.h"
#include "t_stru.h"
#include "t_algs.h"

#include <boost/date_time/posix_time/posix_time.hpp>  

using namespace std;
using namespace ytlib;

class c1 {
public:
	c1() {

		YT_DEBUG_PRINTF("create\n");
	}
	~c1() {
		YT_DEBUG_PRINTF("delete\n");
	}
	const int a=1;
	static int b;
	static const int c;

private:


};
int c1::b = 1;
const int c1::c = 1;



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

	test_sort();
	test_kmp();
	test_strdif();
	test_lgsswd();

	test_avlt();
	test_brt();
	test_bst();
	test_bintree();
	test_heap();


	test_Serialize();
	test_SysInfoTools();
	test_NetLog();
	test_TcpNetAdapter();
	

	test_Complex();
	test_Matrix();
	test_Matrix_c();
	test_tools();

	test_KeyValueFile();
	test_SerializeFile();
	test_XMLFile();
	test_PrjBase();

	test_QueueProcess();
	
	printf("******************end*******************\n");
	getchar();
	
}
