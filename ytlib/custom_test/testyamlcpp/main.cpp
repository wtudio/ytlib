#include <filesystem>
#include <fstream>
#include <string>
#include "ytlib/misc/misc_macro.h"

#include "yaml-cpp/yaml.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  const std::filesystem::path& yaml_file = std::filesystem::absolute("./test.yaml");

  std::ofstream ofile(yaml_file, std::ios::trunc);
  ofile << R"str(
obj1:
  key1: value1
  key2: value2
  array:
  - null_value: 
  - boolean: true
  - integer: 1
  array2: [1,2,3,4,5]
  array3:
  - 1
  - 2
  - 3
obj2:
- key1: value1
- key2: value2
)str";
  ofile.close();

  YAML::Node config;

  try {
    config = YAML::LoadFile(yaml_file.string());
  } catch (const YAML::BadFile& e) {
    DBG_PRINT("YAML::LoadFile failed, %s", e.what());
    return 0;
  }

  try {
    std::string val = config["obj1"]["key1"].as<string>();
    DBG_PRINT("obj1.key1 = %s", val.c_str());

    val = config["obj1"]["key2"].as<string>();
    DBG_PRINT("obj1.key2 = %s", val.c_str());

    std::vector<int> int_arr2 = config["obj1"]["array2"].as<std::vector<int>>();
    DBG_PRINT("obj1.array2 size = %llu", int_arr2.size());

    std::vector<int> int_arr3 = config["obj1"]["array3"].as<std::vector<int>>();
    DBG_PRINT("obj1.array3 size = %llu", int_arr3.size());

  } catch (const std::exception& e) {
    DBG_PRINT("YAML read failed, %s", e.what());
  }

  DBG_PRINT("********************end test*******************");
  return 0;
}
