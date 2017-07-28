#include <ytlib/Common/Util.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/SupportTools/UUID.h>
#include <ytlib/NetTools/TcpNetAdapter.h>

#include "mathtest.h"
#include "filetest.h"
#include "processtest.h"

#include <boost/asio.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>  
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace ytlib;




class testobj {
	T_CLASS_SERIALIZE(&a&b&c)
public:
	
	int32_t a;
	double b;
	std::string c;

};


typedef DataPackage<testobj> myDataPackage;
typedef TcpNetAdapter<testobj> myTcpNetAdapter;

void handel_recv(std::shared_ptr<myDataPackage>& data) {
	printf_s("a_R: %d - %f - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str());
}
void handel_recv2(std::shared_ptr<myDataPackage>& data) {
	printf_s("b_R: %d - %f - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str());
}
int32_t main(int32_t argc, char** argv) {

	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));


	//boost::posix_time::ptime ticTime_global, tocTime_global;
	//ticTime_global = boost::posix_time::microsec_clock::universal_time();

	//tocTime_global = boost::posix_time::microsec_clock::universal_time(); 
	//std::cout << (tocTime_global - ticTime_global).ticks() << "us" << std::endl;



	
	myTcpNetAdapter a(1000, TcpEp(boost::asio::ip::tcp::v4(), 60000),
		std::bind(&handel_recv, std::placeholders::_1));
	a.SetHost(2000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 50000));


	myTcpNetAdapter b(2000, TcpEp(boost::asio::ip::tcp::v4(), 50000),
		std::bind(&handel_recv2, std::placeholders::_1));
	b.SetHost(1000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60000));



	for (int ii = 0; ii < 100; ii++) {
		std::shared_ptr<myDataPackage> packptr = std::shared_ptr<myDataPackage>(new myDataPackage());
		packptr->obj.a = ii;
		packptr->obj.b = ii / 3.256;
		packptr->obj.c = boost::lexical_cast<std::string>(ii * 5);

		assert(a.Send(packptr, 2000));
		Sleep(1000);
	}
	
	for (int ii = 100; ii < 200; ii++) {
		std::shared_ptr<myDataPackage> packptr = std::shared_ptr<myDataPackage>(new myDataPackage());
		packptr->obj.a = ii;
		packptr->obj.b = ii / 3.256;
		packptr->obj.c = boost::lexical_cast<std::string>(ii * 5);

		assert(b.Send(packptr, 1000));
	}

	/*
	test_Complex();
	test_Matrix();
	test_Matrix_c();

	test_KeyValueFile();
	test_SerializeFile();
	test_XMLFile();
	test_PrjBase();

	test_QueueProcess();
	printf_s("*************************************\n");

	*/

	getchar();
	
}
