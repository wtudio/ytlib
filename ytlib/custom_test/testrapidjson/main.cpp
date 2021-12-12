#include <gtest/gtest.h>

#include <string>

#include "rapidjson_tool.hpp"

using namespace std;
using namespace ytlib;

TEST(rapidjson_tool, CheckChild) {
  string json_str = R"str(
{
  "key1":"val1",
  "key2":123,
  "key3":123.0,
  "key4":[],
  "key5":{}
}
)str";

  rapidjson::Document document;
  EXPECT_EQ(document.Parse(json_str.c_str()).HasParseError(), false);
  EXPECT_EQ(document.IsObject(), true);
  EXPECT_EQ(CheckChildStr(document, "key1"), true);
  EXPECT_EQ(CheckChildInt(document, "key2"), true);
  EXPECT_EQ(CheckChildNum(document, "key2"), true);
  EXPECT_EQ(CheckChildDbl(document, "key3"), true);
  EXPECT_EQ(CheckChildNum(document, "key3"), true);
  EXPECT_EQ(CheckChildArr(document, "key4"), true);
  EXPECT_EQ(CheckChildObj(document, "key5"), true);

  EXPECT_EQ(CheckChildStr(document, "key2"), false);
  EXPECT_EQ(CheckChildStr(document, "key10086"), false);
  EXPECT_EQ(CheckChildStr(document["key1"], "key10086"), false);

  EXPECT_STREQ(GetChildStr(document, "key1"), "val1");
  EXPECT_STREQ(GetChildStr(document, "key3"), "");

  EXPECT_EQ(GetChildDbl(document, "key3"), 123.0);
  EXPECT_EQ(GetChildDbl(document, "key2"), 0);

  EXPECT_EQ(GetChildInt(document, "key2"), 123);
  EXPECT_EQ(GetChildInt(document, "key4"), 0);
}

TEST(rapidjson_tool, FindVal) {
  /*
  target val:
  {
    "skey":"strstrstr",
    "ikey":123123,
    "arrkey":[123,456]
  }
  */
  string json_str = R"str(
{
  "skey":"strstrstr_root",
  "arrkey":[123,456],
  "testkey1":{
    "testkey1_1":{
        "skey":"strstrstr_1_1",
        "ikey":123123,
        "arrkey":[123,456]
      },
    "testkey1_2":123
  },
  "testkey2":[
      {
        "skey":"strstrstr2_1",
        "ikey":123123,
        "arrkey":[123,456]
      },
      {
        "skey":"strstrstr_2_2",
        "ikey":123123,
        "arrkey":[123,456]
      },
      {
        "skey":"strstrstr_2_3",
        "ikey":123123,
        "arrkey":[123,456]
      }
  ]
}
)str";
  rapidjson::Document document;
  EXPECT_EQ(document.Parse(json_str.c_str()).HasParseError(), false);

  auto f = [](const rapidjson::Value& val) {
    return (CheckChildStr(val, "skey") && CheckChildInt(val, "ikey") &&
            CheckChildArr(val, "arrkey"));
  };

  std::vector<rapidjson::Value*> vec;
  FindVal(&document, &vec, f);
  EXPECT_EQ(vec.size(), 4);
}

TEST(rapidjson_tool, AddMap) {
  std::map<std::string, std::string> m = {{"aaa", "111"}, {"bbb", "222"}, {"ccc", "333"}};
  rapidjson::Document document;
  rapidjson::Value root(rapidjson::kObjectType);
  AddMap(&root, m, document.GetAllocator());

  string target_json_str = R"str({"aaa":"111","bbb":"222","ccc":"333"})str";

  EXPECT_STREQ(JsonObj2CompactStr(&root).c_str(), target_json_str.c_str());
}

int32_t main(int32_t argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
