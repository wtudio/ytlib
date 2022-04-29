#include <sstream>

#include "ytlib/misc/misc_macro.h"

#include "test.pb.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  testrpc::testreq req;
  req.set_id(1234);
  req.set_name("testname");
  req.set_age(18);

  stringstream ss;

  req.SerializeToOstream(&ss);

  google::protobuf::ShutdownProtobufLibrary();

  DBG_PRINT("********************end test*******************");
  return 0;
}
