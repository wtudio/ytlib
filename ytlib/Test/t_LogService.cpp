#include "t_LogService.h"

namespace ytlib {
void test_NetLog() {
  LoggerServer l(55555);
  l.start();

  InitNetLog(12345, TcpEp(boost::asio::ip::address::from_string("127.0.0.1"), 55555));

  YT_LOG_TRACE << "trace log test";
  YT_LOG_DEBUG << "debug log test";
  YT_LOG_INFO << "info log test";
  YT_LOG_WARNING << "warning log test";

  //Sleep(5000);
  YT_SET_LOG_LEVEL(debug);

  YT_LOG_TRACE << "trace log test";
  YT_LOG_DEBUG << "debug log test";
  YT_LOG_INFO << "info log test";
  YT_LOG_WARNING << "warning log test";

  getchar();
  StopNetLog();
}

}  // namespace ytlib