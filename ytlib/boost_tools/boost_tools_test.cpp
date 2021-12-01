#include <gtest/gtest.h>

#include "serialize.hpp"
#include "xml_tools.hpp"

namespace ytlib {

using std::string;

class CTest {
  T_CLASS_SERIALIZE(&name &age &m &v &dbl_arr)
 public:
  string name;
  uint32_t age;
  std::map<std::string, std::string> m;
  std::vector<uint32_t> v;
  double dbl_arr[5];
};

TEST(BOOST_TOOLS_TEST, SERIALIZE_BASE) {
  CTest obj1;
  obj1.name = "test name";
  obj1.age = 999;
  obj1.m["k1"] = "v1";
  obj1.v.push_back(666);
  obj1.dbl_arr[0] = 1.23456;

  // string
  string re = Serialize(obj1, SerializeType::BinaryType);

  CTest obj2;
  Deserialize(obj2, re, SerializeType::BinaryType);

  ASSERT_STREQ(obj1.name.c_str(), obj2.name.c_str());
  ASSERT_EQ(obj1.age, obj2.age);
  ASSERT_STREQ(obj2.m["k1"].c_str(), "v1");
  ASSERT_EQ(obj1.v[0], obj2.v[0]);
  ASSERT_EQ(obj2.dbl_arr[0], 1.23456);

  // char array
  const uint32_t len = 1024;
  char buf[len];

  uint32_t slen = Serialize(obj1, buf, len, SerializeType::BinaryType);

  CTest obj3;
  Deserialize(obj3, buf, slen, SerializeType::BinaryType);

  ASSERT_STREQ(obj1.name.c_str(), obj3.name.c_str());
  ASSERT_EQ(obj1.age, obj3.age);
  ASSERT_STREQ(obj3.m["k1"].c_str(), "v1");
  ASSERT_EQ(obj1.v[0], obj3.v[0]);
  ASSERT_EQ(obj3.dbl_arr[0], 1.23456);

  // file
  std::filesystem::path p("testTmp/testSerializeFile.txt");

  Serialize(obj1, p, SerializeType::TextType);

  CTest obj4;
  Deserialize(obj4, p, SerializeType::TextType);

  ASSERT_STREQ(obj1.name.c_str(), obj4.name.c_str());
  ASSERT_EQ(obj1.age, obj4.age);
  ASSERT_STREQ(obj4.m["k1"].c_str(), "v1");
  ASSERT_EQ(obj1.v[0], obj4.v[0]);
  ASSERT_EQ(obj4.dbl_arr[0], 1.23456);
}

TEST(BOOST_TOOLS_TEST, XML_TOOLS_BASE) {
  std::filesystem::path p("testTmp/testXMLFile.txt");

  std::map<std::string, std::string> m{{"k1", "v1"}, {"k2", "v2"}};
  boost::property_tree::ptree pt;

  AddXmlSettings(m, pt);
  WriteXmlFile(p, pt);

  std::map<std::string, std::string> m2;
  boost::property_tree::ptree pt2;
  ReadXmlFile(p, pt2);
  ASSERT_TRUE(ReadXmlSettings(pt2, m2));

  ASSERT_EQ(m2.size(), 2);
  ASSERT_STREQ(m2["k1"].c_str(), "v1");
  ASSERT_STREQ(m2["k2"].c_str(), "v2");
}

}  // namespace ytlib
