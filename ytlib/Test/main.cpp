/**
 * @file main.cpp
 * @brief 测试所有接口
 * @details 单元测试所有接口
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#include <ytlib/Common/Util.h>

#include "t_lmath.h"
#include "t_file.h"
#include "t_process.h"
#include "t_net.h"
#include "t_log.h"
#include "t_tools.h"
#include "t_stru.h"
#include "t_algs.h"

#include <boost/core/lightweight_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>  
#include <vector>
#include <map>

using namespace std;
using namespace ytlib;

//测试运行时间小工具
boost::posix_time::ptime ticTime_global;
void setTime(){ ticTime_global = boost::posix_time::microsec_clock::universal_time(); }
string getTime() {
	boost::posix_time::ptime tocTime_global = boost::posix_time::microsec_clock::universal_time();
	return to_string((tocTime_global - ticTime_global).ticks()) + "us";
}

int32_t main(int32_t argc, char** argv) {
	YT_DEBUG_PRINTF("-------------------start-------------------\n");
	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));

	setTime();
	cout << getTime() << endl;

	test_bignum();
	test_graph();

	test_sort();
	test_kmp();
	test_strdif();
	test_strFuns();
	test_lgsswd();

	test_avlt();
	test_brt();
	test_bst();
	test_bintree();
	test_heap();


	test_Serialize();
	test_stl();
	test_urlencode();
	test_LoopTool();
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
	
	YT_DEBUG_PRINTF("******************end*******************\n");
	return boost::report_errors();
}
