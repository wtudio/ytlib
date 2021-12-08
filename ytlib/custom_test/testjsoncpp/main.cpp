#include <string>
#include "ytlib/misc/misc_macro.h"

#include "json/json.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  const std::string str = R"({"Age": 20, "Name": "colin"})";

  Json::Value root;
  Json::CharReaderBuilder builder;
  JSONCPP_STRING err;

  const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  if (!reader->parse(str.c_str(), str.c_str() + str.length(), &root, &err)) {
    DBG_PRINT("error");
    return 0;
  }

  const std::string name = root["Name"].asString();
  DBG_PRINT("name = %s", name.c_str());

  const int age = root["Age"].asInt();
  DBG_PRINT("age = %d", age);

  DBG_PRINT("********************end test*******************");
  return 0;
}
