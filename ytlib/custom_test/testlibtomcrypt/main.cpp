#include <string>
#include "ytlib/misc/misc_macro.h"

#include "tomcrypt.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  std::string str = "this is a test data this is a test data this is a test data";
  const size_t bufszie = 1024;

  unsigned char in_buf[bufszie];
  unsigned long in_bufszie;
  unsigned char out_buf[bufszie];
  unsigned long out_bufszie;

  // base64
  in_bufszie = str.size();
  memcpy(in_buf, str.data(), in_bufszie);

  int ret = base64_encode(in_buf, in_bufszie, out_buf, &(out_bufszie = bufszie));
  DBG_PRINT("base64_encode ret:%d, out_bufszie:%lu, out_buf:%s",
            ret, out_bufszie, std::string(reinterpret_cast<char*>(out_buf), out_bufszie).c_str());

  in_bufszie = out_bufszie;
  memcpy(in_buf, out_buf, in_bufszie);
  ret = base64_decode(in_buf, in_bufszie, out_buf, &(out_bufszie = bufszie));
  DBG_PRINT("base64_encode ret:%d, out_bufszie:%lu, out_buf:%s",
            ret, out_bufszie, std::string(reinterpret_cast<char*>(out_buf), out_bufszie).c_str());

  DBG_PRINT("********************end test*******************");
  return 0;
}
