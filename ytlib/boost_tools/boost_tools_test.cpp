#include <gtest/gtest.h>

#include "KeyValueFile.h"
#include "PrjBase.h"
#include "QueueProcess.h"
#include "Serialize.h"
#include "SerializeFile.h"
#include "XMLFile.h"
#include "log.hpp"
#include "log_svr.hpp"

namespace ytlib {

using std::string;

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

class test_a {
  T_CLASS_SERIALIZE(&s& a& ps& ps2)
 public:
  string s;
  uint32_t a;
  boost::shared_ptr<test_a> ps;
  boost::shared_ptr<test_a> ps2;
};

TEST(BOOST_TOOLS_TEST, SERIALIZE) {
  test_a obj1;
  obj1.s = "dddd";
  obj1.a = 100;
  string re;
  Serialize(obj1, re, SerializeType::BinaryType);

  test_a obj2;
  Deserialize(obj2, re, SerializeType::BinaryType);
}

class testmsg {
 public:
  testmsg(int n) : index(n) {
  }
  int index;
};

class testQueueProcess : public QueueProcess<std::shared_ptr<testmsg> > {
 public:
  testQueueProcess(std::size_t thCount_ = 1, std::size_t queueSize_ = 1000) : QueueProcess(thCount_, queueSize_) {}
  virtual ~testQueueProcess() { stop(); }

 protected:
  void ProcFun(const std::shared_ptr<testmsg>& ptr_) {
    printf("%d\n", ptr_->index);
  }
};

TEST(BOOST_TOOLS_TEST, QueueProcess_BASE) {
  testQueueProcess testprocess;
  testprocess.init();
  testprocess.start();
  for (int ii = 0; ii < 1000; ii++) {
    testprocess.Add(std::shared_ptr<testmsg>(new testmsg(ii)));
  }
  //Sleep(500);
  for (int ii = 1000; ii < 2000; ii++) {
    testprocess.Add(std::shared_ptr<testmsg>(new testmsg(ii)));
  }

  //Sleep(500);

  testprocess.stop();
  printf("*************************************\n");
  //Sleep(500);
  testprocess.start();
  testprocess.init();
  testprocess.start();

  for (int ii = 3000; ii < 4000; ii++) {
    testprocess.Add(std::shared_ptr<testmsg>(new testmsg(ii)));
  }
  testprocess.stop();
}

TEST(BOOST_TOOLS_TEST, KeyValueFile_BASE) {
  KeyValueFile f1;
  f1.NewFile(T_TEXT("t_file/testfile3.txt"));
  std::shared_ptr<std::map<std::string, std::string> > o1 = f1.m_fileobj;
  (*o1)["aaa"] = "bbb";
  (*o1)["ccc"] = "ddd";
  (*o1)["测试1"] = "测试2";
  f1.SaveFile();

  KeyValueFile f2;
  f2.OpenFile(T_TEXT("t_file/testfile3.txt"));
  std::shared_ptr<std::map<std::string, std::string> > o2 = f2.m_fileobj;

  for (std::map<std::string, std::string>::const_iterator itr = o2->begin();
       itr != o2->end(); ++itr) {
    std::cout << itr->first << " = " << itr->second << std::endl;
  }
  (*o2)["测试1"] = "测试3";
  f2.SaveFile(T_TEXT("t_file/testfile4.txt"));
}

class SFTestObj {
  T_CLASS_SERIALIZE(&s1& s2& i1& i2)
 public:
  std::string s1;
  std::string s2;
  int32_t i1;
  int32_t i2;
};
typedef SerializeFile<SFTestObj> SFTestFile;

TEST(BOOST_TOOLS_TEST, SerializeFile_BASE) {
  SFTestFile f1;
  f1.NewFile(T_TEXT("t_file/testfile.txt"));
  std::shared_ptr<SFTestObj> o = f1.m_fileobj;
  o->s1 = "sssadafasf";
  o->s2 = "测试";
  o->i1 = 1067;
  o->i2 = 164;
  f1.SaveFile();
  tcout << f1.GetFileName() << std::endl;
  tcout << f1.GetFileParentPath() << std::endl;

  SFTestFile f2;
  f2.OpenFile(T_TEXT("t_file/testfile.txt"));
  f2.SaveFile(T_TEXT("t_file/testfile2.txt"));
}

TEST(BOOST_TOOLS_TEST, XMLFile_BASE) {
  XMLFile f1;
  f1.NewFile();

  f1.SaveFile(T_TEXT("t_file/TrunkCenter2.xml"));
}

TEST(BOOST_TOOLS_TEST, PrjFile_BASE) {
  PrjFile f1;
  f1.NewFile();
  f1.setPrjName(T_TEXT("testprj2"));
  f1.SaveFile(T_TEXT("t_file/testprj2.prj"));
}

}  // namespace ytlib
