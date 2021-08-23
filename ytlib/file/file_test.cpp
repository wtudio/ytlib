#include <gtest/gtest.h>

#include "fileobj_kv.hpp"
#include "ytlib/misc/stl_util.hpp"

namespace ytlib {

TEST(FILEOBJ_KV_TEST, ParseFileObj_test) {
  struct TestCase {
    std::string name;

    std::string file_path;
    std::string file_data;

    std::map<std::string, std::string> want_map;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .file_path = "testKeyValueFile1_1.txt",
      .file_data = R"str(
k1 = v1
k2 =v2
 k3= v3
)str",
      .want_map = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .file_path = "testKeyValueFile1_2.txt",
      .file_data = R"str(
# test
k1 = v1 #test
k2 =v2 # test ## ss
# k=v
 k3= v3#v3
)str",
      .want_map = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}}});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .file_path = "testKeyValueFile1_3.txt",
      .file_data = R"str(
k1 = v1
k2 =v2
k3= v3x

 k3= v3

k4
 = v4

k5 =
)str",
      .want_map = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"k5", ""}}});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    std::ofstream ofile(test_cases[ii].file_path, std::ios::trunc);
    ASSERT_TRUE(ofile) << "Test " << test_cases[ii].name << " failed, index " << ii
                       << ", can not create test file, path " << test_cases[ii].file_path;

    ofile << test_cases[ii].file_data;
    ofile.close();

    KeyValueFile test_kv_file;
    test_kv_file.OpenFile(test_cases[ii].file_path);

    auto map_ptr = test_kv_file.GetObjPtr();
    ASSERT_TRUE(map_ptr);
    EXPECT_STREQ(Map2Str(*map_ptr).c_str(), Map2Str(test_cases[ii].want_map).c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

TEST(FILEOBJ_KV_TEST, SaveFileObj_test) {
  struct TestCase {
    std::string name;

    std::string file_path;
    std::map<std::string, std::string> kv_map;

    std::string want_file_data;
  };
  std::vector<TestCase> test_cases;
  test_cases.emplace_back(TestCase{
      .name = "case 1",
      .file_path = "testKeyValueFile2_1.txt",
      .kv_map = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}},
      .want_file_data = R"str(k1 = v1
k2 = v2
k3 = v3
)str"});
  test_cases.emplace_back(TestCase{
      .name = "case 2",
      .file_path = "testKeyValueFile2_2.txt",
      .kv_map = {{"k1", "v1"}, {"k2", "v2"}, {"k3", "v3"}, {"", "v4"}},
      .want_file_data = R"str(k1 = v1
k2 = v2
k3 = v3
)str"});
  test_cases.emplace_back(TestCase{
      .name = "case 3",
      .file_path = "testKeyValueFile2_3.txt",
      .kv_map = {},
      .want_file_data = ""});

  for (size_t ii = 0; ii < test_cases.size(); ++ii) {
    KeyValueFile test_kv_file;
    test_kv_file.NewFile(test_cases[ii].file_path);

    auto map_ptr = test_kv_file.GetObjPtr();
    ASSERT_TRUE(map_ptr);
    *map_ptr = test_cases[ii].kv_map;

    test_kv_file.SaveFile();

    std::ifstream infile(test_cases[ii].file_path, std::ios::in);
    ASSERT_TRUE(infile) << "Test " << test_cases[ii].name << " failed, index " << ii
                        << ", can not open test file, path " << test_cases[ii].file_path;

    std::stringstream buf;
    buf << infile.rdbuf();
    std::string str = buf.str();

    infile.close();

    EXPECT_STREQ(str.c_str(), test_cases[ii].want_file_data.c_str())
        << "Test " << test_cases[ii].name << " failed, index " << ii;
  }
}

}  // namespace ytlib
