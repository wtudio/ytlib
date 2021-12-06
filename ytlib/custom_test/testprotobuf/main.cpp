#include <sstream>
#include <vector>
#include "ytlib/misc/misc_macro.h"

#include "gen/test.pb.h"

using namespace std;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  GOOGLE_PROTOBUF_VERIFY_VERSION;

  test::testmsg msg;
  msg.set_id(1234);
  msg.set_name("testname");
  msg.set_age(18);

  stringstream ss;

  msg.SerializeToOstream(&ss);

  test::testmsg msg2;
  msg2.ParseFromIstream(&ss);
  DBG_PRINT("msg2.id() = %llu", msg2.id());
  DBG_PRINT("msg2.name() = %s", msg2.name().c_str());
  DBG_PRINT("msg2.age() = %d", msg2.age());

  google::protobuf::ShutdownProtobufLibrary();
  DBG_PRINT("********************end test*******************");
  return 0;
}
