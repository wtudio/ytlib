#include <benchmark/benchmark.h>

#include <thread>

#include "asio_net_log_cli.hpp"
#include "asio_net_log_svr.hpp"
#include "asio_tools.hpp"
#include "boost_log.hpp"

namespace ytlib {
class NetLogFixture {
 public:
  NetLogFixture() {
    DBG_PRINT("NetLogFixture setup");

    AsioDebugTool::Ins().Reset();

    // log cli
    auto log_cli_sys_ptr = std::make_shared<AsioExecutor>(1);
    log_cli_sys_ptr_ = log_cli_sys_ptr;

    AsioNetLogClient::Cfg net_log_client_cfg;
    net_log_client_cfg.svr_ep = boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 50009};
    auto net_log_cli_ptr = std::make_shared<AsioNetLogClient>(log_cli_sys_ptr->IO(), net_log_client_cfg);
    log_cli_sys_ptr->RegisterSvrFunc(std::function<void()>(),
                                     [net_log_cli_ptr] { net_log_cli_ptr->Stop(); });

    YTBLCtr::Ins().EnableNetLog(net_log_cli_ptr);
    YTBL_SET_LEVEL(trace);

    t_cli_ = std::make_shared<std::thread>([log_cli_sys_ptr] {
      log_cli_sys_ptr->Start();
      log_cli_sys_ptr->Join();
      DBG_PRINT("log_cli_sys_ptr exit");
    });
  }
  ~NetLogFixture() {
    DBG_PRINT("NetLogFixture teardown");

    log_cli_sys_ptr_->Stop();
    t_cli_->join();
  }

 private:
  std::shared_ptr<AsioExecutor> log_cli_sys_ptr_;

  std::shared_ptr<std::thread> t_cli_;
};

static NetLogFixture net_log_fixture;

static void BM_NetLog(benchmark::State& state) {
  for (auto _ : state) {
    YTBL_TRACE << "test trace log" << std::endl;
    YTBL_DEBUG << "test debug log" << std::endl;
    YTBL_INFO << "test info log" << std::endl;
    YTBL_WARN << "test warning log" << std::endl;
    YTBL_ERROR << "test error log" << std::endl;
    YTBL_FATAL << "test fatal log" << std::endl;
  }
}
BENCHMARK(BM_NetLog);

}  // namespace ytlib

BENCHMARK_MAIN();
