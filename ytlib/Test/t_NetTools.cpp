#include "t_NetTools.h"

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace boost::asio;
using namespace std;

namespace ytlib {
class testobj {
  T_CLASS_SERIALIZE(&a& b& c& d)
 public:
  int32_t a;
  double b;
  std::string c;
  std::string d;
};

typedef DataPackage<testobj> myDataPackage;
typedef TcpNetAdapter<testobj> myTcpNetAdapter;

bool iserr = true;

void handel_recv(std::shared_ptr<myDataPackage>& data) {
  printf("a_R: %d - %f - %s - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str(),
         string(data->map_datas["data1"].buf.get(), 10).c_str());
  iserr &= (data->map_datas["data1"].buf_size == 1000);
  //iserr &= (data->map_datas["data2"].buf_size == 1000);

  //iserr &= (data->map_files["file1"] == "testprj2.prj");
  //iserr &= (data->map_files["file2"] == "TrunkCenter2.xml");
  //iserr &= (data->map_files["file3"] == "testfile.txt");
}
void handel_recv2(std::shared_ptr<myDataPackage>& data) {
  printf("b_R: %d - %f - %s - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str(),
         string(data->map_datas["data1"].buf.get(), 10).c_str());
  iserr &= (data->map_datas["data1"].buf_size == 1000);
  //iserr &= (data->map_datas["data2"].buf_size == 1000);

  //iserr &= (data->map_files["file1"] == "testprj2.prj");
  //iserr &= (data->map_files["file2"] == "TrunkCenter2.xml");
  //iserr &= (data->map_files["file3"] == "testfile.txt");
}
void handel_recv3(std::shared_ptr<myDataPackage>& data) {
  printf("c_R: %d - %f - %s - %s\n", data->obj.a, data->obj.b, data->obj.c.c_str(),
         string(data->map_datas["data1"].buf.get(), 10).c_str());
  iserr &= (data->map_datas["data1"].buf_size == 1000);
  //iserr &= (data->map_datas["data2"].buf_size == 1000);

  //iserr &= (data->map_files["file1"] == "testprj2.prj");
  //iserr &= (data->map_files["file2"] == "TrunkCenter2.xml");
  //iserr &= (data->map_files["file3"] == "testfile.txt");
}

void send_fun(myTcpNetAdapter* p, std::vector<uint32_t> id) {
  uint32_t count = 0;
  std::string s;
  for (int ii = 0; ii < 1000; ii++) {
    s += "0123456789";
  }

  boost::shared_array<char> data = boost::shared_array<char>(new char[1000]);
  for (int ii = 0; ii < 1000; ++ii) {
    data[ii] = '0' + ii % 10;
  }

  string file1("D:/project/ytlib/ytlib/build/bin/Debug/t_file/testprj2.prj");
  string file2("D:/project/ytlib/ytlib/build/bin/Debug/t_file/TrunkCenter2.xml");
  string file3("D:/project/ytlib/ytlib/build/bin/Debug/t_file/testfile.txt");

  for (int ii = 0; ii < 100; ++ii) {
    std::shared_ptr<myDataPackage> packptr = std::shared_ptr<myDataPackage>(new myDataPackage());
    packptr->obj.a = ii;
    packptr->obj.b = ii / 3.256;
    packptr->obj.c = std::to_string(ii * 5);
    packptr->obj.d = s;

    sharedBuf mybuf;
    mybuf.buf = data;
    mybuf.buf_size = 1000;
    packptr->map_datas["data1"] = mybuf;

    if (ii % 5 == 0) {
      packptr->map_datas["data2"] = mybuf;

      packptr->map_files["file1"] = file1;
      packptr->map_files["file2"] = file2;
      packptr->map_files["file3"] = file3;
    }

    if (!(p->Send(packptr, id))) {
      ++count;
    }
    //Sleep(1000);
  }
  printf("----------------------get %d failed--------------------\n", count);
  iserr &= (count == 0);
}

void test_TcpNetAdapter() {
  if (true) {
    boost::posix_time::ptime ticTime_global, tocTime_global;
    ticTime_global = boost::posix_time::microsec_clock::universal_time();

    myTcpNetAdapter* a = new myTcpNetAdapter(1000, 60001,
                                             std::bind(&handel_recv, std::placeholders::_1), T_TEXT("a/recv"), T_TEXT("a/send"));
    iserr &= (a->start());
    a->SetHost(1000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60001));
    a->SetHost(2000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60002));
    a->SetHost(3000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60003));

    myTcpNetAdapter* b = new myTcpNetAdapter(2000, 60002,
                                             std::bind(&handel_recv2, std::placeholders::_1), T_TEXT("b/recv"), T_TEXT("b/send"));
    iserr &= (b->start());
    b->SetHost(1000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60001));
    b->SetHost(2000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60002));
    b->SetHost(3000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60003));

    myTcpNetAdapter* c = new myTcpNetAdapter(3000, 60003,
                                             std::bind(&handel_recv3, std::placeholders::_1), T_TEXT("c/recv"), T_TEXT("c/send"));
    iserr &= (c->start());
    c->SetHost(1000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60001));
    c->SetHost(2000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60002));
    c->SetHost(3000, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 60003));

    std::vector<uint32_t> dst1 = {2000, 3000};

    boost::thread_group m_RunThreads;
    for (int ii = 0; ii < 5; ++ii) {
      m_RunThreads.create_thread(std::bind(&send_fun, a, std::vector<uint32_t>{2000, 3000}));
      //m_RunThreads.create_thread(std::bind(&send_fun, a, 3000));

      //Sleep(100);
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      m_RunThreads.create_thread(std::bind(&send_fun, b, std::vector<uint32_t>{1000, 3000}));
      //m_RunThreads.create_thread(std::bind(&send_fun, b, 3000));

      //Sleep(100);
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
      m_RunThreads.create_thread(std::bind(&send_fun, c, std::vector<uint32_t>{2000, 1000}));
      //m_RunThreads.create_thread(std::bind(&send_fun, c, 2000));
    }
    m_RunThreads.join_all();

    tocTime_global = boost::posix_time::microsec_clock::universal_time();
    printf("-----------------count : %lld us----------------------\n", (tocTime_global - ticTime_global).ticks());
    //std::cout << (tocTime_global - ticTime_global).ticks() << "us" << std::endl;

    getchar();

    delete a;
    //Sleep(500);
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    delete b;
    //Sleep(500);
    boost::this_thread::sleep(boost::posix_time::milliseconds(500));
    delete c;
  }
  if (!iserr) {
    printf("get err!!!\n");
  }
  //return iserr;
}

}  // namespace ytlib