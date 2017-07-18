#include <ytlib/Common/Util.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/SupportTools/UUID.h>

#include "mathtest.h"
#include "filetest.h"
#include "processtest.h"


using namespace std;
using namespace wtlib;


int32_t main(int32_t argc, char** argv) {

	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));



	test_Complex();
	test_Matrix();
	test_Matrix_c();

	test_KeyValueFile();
	test_SerializeFile();
	test_XMLFile();
	test_PrjBase();

	test_QueueProcess();
	printf_s("*************************************\n");

	

	getchar();
	
}
