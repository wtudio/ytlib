#include <gtest/gtest.h>

#include "log.hpp"
#include "log_macro.hpp"
#include "print_ctr.hpp"

#include "rotate_file_writer.hpp"

namespace ytlib {

TEST(LOG_TEST, log_BASE) {
  Log::Ins().SetLevel(LOG_LEVEL::L_FATAL);
  EXPECT_EQ(Log::Ins().Level(), LOG_LEVEL::L_FATAL);
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
  PrintCtr::Ins().SetPrint(true);
  ASSERT_TRUE(PrintCtr::Ins().IfPrint());

  TestObj obj = {20, "testname"};
  std::string struct_str = R"str(test msg:
age:20
name:testname

)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintStruct("test msg", obj).c_str(), struct_str.c_str());

  std::vector<std::string> v = {"val1", "val2", "val3\nval3val3"};
  std::string v_str = R"str(test vec:
vec size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintVec("test vec", v).c_str(), v_str.c_str());

  std::set<std::string> s = {"val1", "val2", "val3\nval3val3"};
  std::string s_str = R"str(test set:
set size = 3
[index=0]:val1
[index=1]:val2
[index=2]:
val3
val3val3
)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintSet("test set", s).c_str(), s_str.c_str());

  std::map<std::string, std::string> m = {{"key1", "val1"}, {"key2", "val2"}, {"key3", "val3\nval3val3"}};
  std::string m_str = R"str(test map:
map size = 3
[index=0]:
  [key]:key1
  [val]:val1
[index=1]:
  [key]:key2
  [val]:val2
[index=2]:
  [key]:key3
  [val]:
val3
val3val3
)str";
  EXPECT_STREQ(PrintCtr::Ins().PrintMap("test map", m).c_str(), m_str.c_str());
}

}  // namespace ytlib
