#include <filesystem>
#include <fstream>
#include <string>
#include "ytlib/misc/misc_macro.h"

#include "tinyxml2.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  const std::filesystem::path& yaml_file = std::filesystem::absolute("./test.xml");

  std::ofstream ofile(yaml_file, std::ios::trunc);
  ofile << R"str(
<document>
	<data>1</data>
	<data>2</data>
	<data>3</data>
</document>
)str";
  ofile.close();

  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError error = doc.LoadFile(yaml_file.string().c_str());

  if (error != tinyxml2::XMLError::XML_SUCCESS) {
    DBG_PRINT("LoadFile error %d", error);
    return 0;
  }

  doc.SaveFile(yaml_file.string().c_str());

  DBG_PRINT("********************end test*******************");
  return 0;
}
