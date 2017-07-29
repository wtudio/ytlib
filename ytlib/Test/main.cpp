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
using namespace boost::asio;



class testobj {
	T_CLASS_SERIALIZE(&a&b&c&d)
public:
	
	int32_t a;
	double b;
	std::string c;
	std::string d;
};



typedef DataPackage<testobj> myDataPackage;
typedef TcpNetAdapter<testobj> myTcpNetAdapter;

void handel_recv(std::shared_ptr<myDataPackage>& data) {
	printf_s("a_R: %d - %f - %s - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str(), 
		string(data->map_datas["data1"].buf.get(), 10).c_str());
	assert(data->map_datas["data1"].buf_size == 1000);
	assert(data->map_datas["data2"].buf_size == 1000);

	assert(data->map_files["file1"] == "testprj2.prj");
	assert(data->map_files["file2"] == "TrunkCenter2.xml");
	assert(data->map_files["file3"] == "testfile.txt");
}
void handel_recv2(std::shared_ptr<myDataPackage>& data) {
	printf_s("b_R: %d - %f - %s - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str(),
		string(data->map_datas["data1"].buf.get(), 10).c_str());
	assert(data->map_datas["data1"].buf_size == 1000);
	assert(data->map_datas["data2"].buf_size == 1000);

	assert(data->map_files["file1"] == "testprj2.prj");
	assert(data->map_files["file2"] == "TrunkCenter2.xml");
	assert(data->map_files["file3"] == "testfile.txt");
}
void handel_recv3(std::shared_ptr<myDataPackage>& data) {
	printf_s("c_R: %d - %f - %s - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str(),
		string(data->map_datas["data1"].buf.get(), 10).c_str());
	assert(data->map_datas["data1"].buf_size == 1000);
	assert(data->map_datas["data2"].buf_size == 1000);

	assert(data->map_files["file1"] == "testprj2.prj");
	assert(data->map_files["file2"] == "TrunkCenter2.xml");
	assert(data->map_files["file3"] == "testfile.txt");
}

void send_fun(myTcpNetAdapter* p,std::vector<uint32_t> id) {
	uint32_t count = 0;
	std::string s;
	for (int ii = 0; ii < 1000; ii++) {
		s += "0123456789";
	}
	
	boost::shared_array<char> data = boost::shared_array<char>( new char[1000]);
	for (int ii = 0; ii < 1000; ++ii) {
		data[ii] = '0' + ii % 10;
	}

	string file1("D:/project/ytlib/ytlib/build/bin/Debug/filetest/testprj2.prj");
	string file2("D:/project/ytlib/ytlib/build/bin/Debug/filetest/TrunkCenter2.xml");
	string file3("D:/project/ytlib/ytlib/build/bin/Debug/filetest/testfile.txt");

	for (int ii = 0; ii < 100; ++ii) {
		std::shared_ptr<myDataPackage> packptr = std::shared_ptr<myDataPackage>(new myDataPackage());
		packptr->obj.a = ii;
		packptr->obj.b = ii / 3.256;
		packptr->obj.c = boost::lexical_cast<std::string>(ii * 5);
		packptr->obj.d = s;
		shared_buf mybuf;
		mybuf.buf = data;
		mybuf.buf_size = 1000;
		packptr->map_datas["data1"] = mybuf;
		packptr->map_datas["data2"] = mybuf;

		packptr->map_files["file1"] = file1;
		packptr->map_files["file2"] = file2;
		packptr->map_files["file3"] = file3;
		if (!(p->Send(packptr, id))) {
			++count;
		}
		//Sleep(1000);
	}
	printf_s("----------------------get %d failed--------------------\n", count);
	assert(count == 0);
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

	if (true) {
		myTcpNetAdapter* a = new myTcpNetAdapter(1000, TcpEp(boost::asio::ip::tcp::v4(), 60001),
			std::bind(&handel_recv, std::placeholders::_1), T_TEXT("a/recv"), T_TEXT("a/send"));
		assert(a->startListener());
		a->SetHost(1000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60001));
		a->SetHost(2000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60002));
		a->SetHost(3000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60003));


		myTcpNetAdapter* b=new myTcpNetAdapter(2000, TcpEp(boost::asio::ip::tcp::v4(), 60002),
			std::bind(&handel_recv2, std::placeholders::_1), T_TEXT("b/recv"), T_TEXT("b/send"));
		assert(b->startListener());
		b->SetHost(1000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60001));
		b->SetHost(2000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60002));
		b->SetHost(3000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60003));

		myTcpNetAdapter* c=new myTcpNetAdapter(3000, TcpEp(boost::asio::ip::tcp::v4(), 60003),
			std::bind(&handel_recv3, std::placeholders::_1), T_TEXT("c/recv"), T_TEXT("c/send"));
		assert(c->startListener());
		c->SetHost(1000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60001));
		c->SetHost(2000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60002));
		c->SetHost(3000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60003));

		std::vector<uint32_t> dst1 = { 2000,3000 };


		boost::thread_group m_RunThreads;
		for (int ii = 0; ii < 5; ++ii) {
			m_RunThreads.create_thread(std::bind(&send_fun, a, std::vector<uint32_t>{2000, 3000}));
			//m_RunThreads.create_thread(std::bind(&send_fun, a, 3000));

			Sleep(100);
			m_RunThreads.create_thread(std::bind(&send_fun, b, std::vector<uint32_t>{1000, 3000}));
			//m_RunThreads.create_thread(std::bind(&send_fun, b, 3000));

			Sleep(100);

			m_RunThreads.create_thread(std::bind(&send_fun, c, std::vector<uint32_t>{2000, 1000}));
			//m_RunThreads.create_thread(std::bind(&send_fun, c, 2000));
		}
		m_RunThreads.join_all();


		getchar();

		delete a;
		Sleep(500);
		delete b;
		Sleep(500);
		delete c;
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
	*/
	printf_s("*************************************\n");

	

	getchar();
	
}
