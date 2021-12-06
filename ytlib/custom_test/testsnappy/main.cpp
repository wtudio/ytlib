#include <string>
#include "ytlib/misc/misc_macro.h"

#include "snappy.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  std::string buf;

  std::string s1 = "test msg test msg test msg test msg";
  DBG_PRINT("s1 size : %llu", s1.size());

  snappy::Compress(s1.c_str(), s1.size(), &buf);
  DBG_PRINT("buf size : %llu", buf.size());

  std::string s2;
  snappy::Uncompress(buf.c_str(), buf.size(), &s2);
  DBG_PRINT("s2 size : %llu", s2.size());
  DBG_PRINT("s2 : %s", s2.c_str());

  DBG_PRINT("********************end test*******************");
  return 0;
}
