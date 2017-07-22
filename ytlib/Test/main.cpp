#include <ytlib/Common/Util.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/SupportTools/UUID.h>

#include "mathtest.h"
#include "filetest.h"
#include "processtest.h"

#include <boost/asio.hpp>

using namespace std;
using namespace ytlib;

class test2 {
	T_CLASS_SERIALIZE(&c&d)
public:
	int c;
	std::string d;
};

class test {
	T_CLASS_SERIALIZE(&a&b&p)
public:
	int a;
	std::string b;
	std::shared_ptr<test2> p;
};

int32_t main(int32_t argc, char** argv) {

	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));

	test aaa;
	aaa.a = 100;
	aaa.b = "ssssssw";
	aaa.p = std::shared_ptr<test2>(new test2);
	aaa.p->c = 333;
	aaa.p->d = "fffff";

	string s;
	Serialize(aaa, s);

	cout << s << endl;
	
	test bbb;
	Deserialize(bbb, s);



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
