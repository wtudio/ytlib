#include <gtest/gtest.h>

#include "log.hpp"
#include "log_macro.hpp"
#include "print_helper.hpp"

#include "rotate_file_writer.hpp"

namespace ytlib {

TEST(LOG_TEST, log_BASE) {
  Log::Ins().SetLevel(LOG_LEVEL::L_FATAL);
  EXPECT_EQ(Log::Ins().Level(), LOG_LEVEL::L_FATAL);

  std::map<std::string, std::string> cfg{
      {"path", "log"},
      {"filename", "test.log"},
      {"rotate_type", "time"},
      {"max_file_size_m", "1"},
      {"max_file_num", "5"}};
  Log::Ins().AddWriter(GetRotateFileWriter(cfg));
  Log::Ins().Init(LOG_LEVEL::L_TRACE);

  for (int ii = 0; ii < 1000; ++ii) {
    YT_TRACE("test YT_TRACE");
    YT_DEBUG("test YT_DEBUG");
    YT_INFO("test YT_INFO");
    YT_WARN("test YT_WARN");
    YT_ERROR("test YT_ERROR");
    YT_FATAL("test YT_FATAL");
  }
}

struct TestObj {
  int age;
  std::string name;

  friend std::ostream& operator<<(std::ostream& out, const TestObj& obj) {
    out << "age:" << obj.age << "\n";
    out << "name:" << obj.name << "\n";
    return out;
  }
};

TEST(LOG_TEST, PrintHelper_BASE) {
  PrintHelper::Ins().SetPrint(true);
  ASSERT_TRUE(PrintHelper::Ins().IfPrint());

  TestObj obj = {20, "testname"};
  std::string struct_str = R"str(test msg:
age:20
name:testname
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintStruct("test msg", obj).c_str(), struct_str.c_str());

  std::vector<std::string> v = {"val1", "val2", "val3\nval3val3"};
  std::string v_str = R"str(test vec:
vec size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintVec("test vec", v).c_str(), v_str.c_str());

  std::set<std::string> s = {"val1", "val2", "val3\nval3val3"};
  std::string s_str = R"str(test set:
set size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintSet("test set", s).c_str(), s_str.c_str());

  std::map<std::string, std::string> m = {{"key1", "val1"}, {"key2", "val2"}, {"key3", "val3\nval3val3"}};
  std::string m_str = R"str(test map:
map size = 3
[index=0, key=key1]:val1
[index=1, key=key2]:val2
[index=2, key=key3]:
val3
val3val3
)str";
  ASSERT_STREQ(PrintHelper::Ins().PrintMap("test map", m).c_str(), m_str.c_str());
}

}  // namespace ytlib
