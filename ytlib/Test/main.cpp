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
#include <vector>
#include <map>

using namespace std;
using namespace ytlib;

class c1 {
public:
	c1():index(b++){

		YT_DEBUG_PRINTF("create:%d\n", index);

	}
	c1(const c1& val) :index(b++) {
		YT_DEBUG_PRINTF("copy:%d to %d\n", val.index, index);
	}
	c1(c1&& val):index(val.index) {
		YT_DEBUG_PRINTF("move:%d\n", index);
	}

	~c1() {
		YT_DEBUG_PRINTF("delete:%d\n", index);
	}

	int index;
	const int a=1;
	static int b;
	static const int c;

private:


};
int c1::b = 1;
const int c1::c = 1;

class w1 {
	char a;
	char b;
	uint32_t c;
};
class w2 {
	char a;
	uint32_t c;
	char b;
};

int32_t main(int32_t argc, char** argv) {
	YT_DEBUG_PRINTF("-------------------start-------------------\n");
	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));

	map<int, string> m;
	m[1] = "111";
	m[2] = "222";
	pair<map<int, string>::iterator, bool> re = m.insert(pair<int, string>(1, "123"));
	if (!re.second) {
		re.first->second = "123";
	}

	multimap<int, string> m2;
	m2.insert(pair<int, string>(1, "001"));
	m2.insert(pair<int, string>(1, "0001"));
	m2.insert(pair<int, string>(1, "00001"));
	m2.insert(pair<int, string>(2, "002"));
	m2.insert(pair<int, string>(2, "0002"));
	m2.insert(pair<int, string>(2, "00002"));
	pair<multimap<int, string>::iterator, multimap<int, string>::iterator> re2 = m2.equal_range(1);
	for (; re2.first != re2.second; ++re2.first) {
		cout << re2.first->first << " " << re2.first->second << endl;
	}

	//boost::posix_time::ptime ticTime_global, tocTime_global;
	//ticTime_global = boost::posix_time::microsec_clock::universal_time();

	//tocTime_global = boost::posix_time::microsec_clock::universal_time(); 
	//std::cout << (tocTime_global - ticTime_global).ticks() << "us" << std::endl;
	
	//setTime();
	//cout << getTime() << endl;
	test_bignum();
	test_graph();

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
