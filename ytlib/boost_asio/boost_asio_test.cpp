#include <gtest/gtest.h>

#include "asio_tools.hpp"
#include "boost_log.hpp"
#include "net_log.hpp"
#include "net_util.hpp"
#include "ytlib/misc/misc_macro.h"

namespace ytlib {

using namespace boost::asio;
using namespace std;

TEST(BOOST_ASIO_TEST, UTIL) {
  uint16_t port = GetUsablePort();
  ASSERT_TRUE(CheckPort(port));

  char buf[4];
  uint32_t n = 123456789;
  SetBufFromNum(buf, n);
  ASSERT_EQ(GetNumFromBuf(buf), n);
}

TEST(BOOST_ASIO_TEST, LOG) {
  auto cli_sys_ptr = std::make_shared<AsioExecutor>(1);
  auto svr1_sys_ptr = std::make_shared<AsioExecutor>(1);
  auto svr2_sys_ptr = std::make_shared<AsioExecutor>(1);

  // cli
  auto net_log_cli_ptr = std::make_shared<NetLogClient>(cli_sys_ptr->IO(), TcpEp{IPV4({127, 0, 0, 1}), 50001});
  cli_sys_ptr->RegisterSvrFunc(std::function<void()>(),
                               [&net_log_cli_ptr] { net_log_cli_ptr->Stop(); });

  YTBLCtr::Ins().EnableConsoleLog();
  YTBLCtr::Ins().EnableFileLog("./test");
  YTBLCtr::Ins().EnableNetLog(net_log_cli_ptr);

  thread t_cli([cli_sys_ptr] {
    cli_sys_ptr->Start();
    cli_sys_ptr->Join();
    DBG_PRINT("cli_sys_ptr exit");
  });

  //svr1
  thread t_svr1([svr1_sys_ptr] {
    auto lgsvr_ptr = std::make_shared<LogSvr>(svr1_sys_ptr->IO(), LogSvrCfg());
    svr1_sys_ptr->RegisterSvrFunc([&lgsvr_ptr] { lgsvr_ptr->Start(); },
                                  [&lgsvr_ptr] { lgsvr_ptr->Stop(); });

    svr1_sys_ptr->Start();
    svr1_sys_ptr->Join();
    DBG_PRINT("svr1_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  YTBL_SET_LEVEL(info);

  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(1));
  svr1_sys_ptr->Stop();
  t_svr1.join();

  // svr2
  thread t_svr2([svr2_sys_ptr] {
    LogSvrCfg cfg;
    cfg.port = 50001;
    cfg.log_path = "./log2";
    cfg.timer_dt = 1;
    cfg.max_no_data_time = 3;
    auto lgsvr_ptr = std::make_shared<LogSvr>(svr2_sys_ptr->IO(), cfg);
    svr2_sys_ptr->RegisterSvrFunc([&lgsvr_ptr] { lgsvr_ptr->Start(); },
                                  [&lgsvr_ptr] { lgsvr_ptr->Stop(); });
    svr2_sys_ptr->Start();
    svr2_sys_ptr->Join();
    DBG_PRINT("svr2_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(5));

  YTBL_TRACE << "test trace log" << std::endl;
  YTBL_DEBUG << "test debug log" << std::endl;
  YTBL_INFO << "test info log" << std::endl;
  YTBL_WARN << "test warning log" << std::endl;
  YTBL_ERROR << "test error log" << std::endl;
  YTBL_FATAL << "test fatal log" << std::endl;

  std::this_thread::sleep_for(std::chrono::seconds(1));
  svr2_sys_ptr->Stop();
  t_svr2.join();

  cli_sys_ptr->Stop();
  t_cli.join();
}

/*
TEST(BOOST_TOOLS_TEST, LOG) {
  LoggerServer l(55555);
  l.start();

  InitNetLog(12345, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 55555));

  YT_LOG_TRACE << "trace log test";
  YT_LOG_DEBUG << "debug log test";
  YT_LOG_INFO << "info log test";
  YT_LOG_WARNING << "warning log test";

  //Sleep(5000);
  YT_SET_LOG_LEVEL(debug);

  YT_LOG_TRACE << "trace log test";
  YT_LOG_DEBUG << "debug log test";
  YT_LOG_INFO << "info log test";
  YT_LOG_WARNING << "warning log test";

  // getchar();
  StopNetLog();
}


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

TEST(BOOST_TOOLS_TEST, TcpNetAdapter_BASE) {
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
}
*/
}  // namespace ytlib
