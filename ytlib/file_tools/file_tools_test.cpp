#include <gtest/gtest.h>

#include "fileobj_kv.hpp"

namespace ytlib {

TEST(FILE_TOOLS_TEST, KeyValueFile_BASE) {
  std::string test_file_path = "testTmp/testKeyValueFile.txt";

  KeyValueFile f1;
  f1.NewFile(test_file_path);
  auto& map1 = *(f1.GetObjPtr());
  map1["k1"] = "v1";
  map1["k2"] = "v2";
  map1["k3"] = "v3";
  map1[""] = "v4";
  map1["k5"] = "";
  f1.SaveFile();

  KeyValueFile f2;
  f2.OpenFile(test_file_path);
  auto& map2 = *(f2.GetObjPtr());
  ASSERT_EQ(map2.size(), 4);
  ASSERT_STREQ(map2["k1"].c_str(), "v1");
  ASSERT_STREQ(map2["k2"].c_str(), "v2");
  ASSERT_STREQ(map2["k3"].c_str(), "v3");
  ASSERT_STREQ(map2["k5"].c_str(), "");

  map2["k1"] = "v11";
  std::string test_file_path2 = "testTmp/testKeyValueFile2.txt";
  f2.SaveFile(test_file_path2);

  f2.OpenFile();
  auto& map3 = *(f2.GetObjPtr());
  ASSERT_NE(&map2, &map3);
  ASSERT_EQ(map3.size(), 4);
  ASSERT_STREQ(map3["k1"].c_str(), "v11");
  ASSERT_STREQ(map3["k2"].c_str(), "v2");
  ASSERT_STREQ(map3["k3"].c_str(), "v3");
  ASSERT_STREQ(map3["k5"].c_str(), "");
}

}  // namespace ytlib
