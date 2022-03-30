#include <benchmark/benchmark.h>

#include <thread>

#include "asio_tools.hpp"
#include "boost_log.hpp"
#include "net_log.hpp"

namespace ytlib {
class NetLogFixture {
 public:
  NetLogFixture() {
    DBG_PRINT("NetLogFixture setup");

    // log svr
    auto log_svr_sys_ptr = std::make_shared<AsioExecutor>(8);
    log_svr_sys_ptr_ = log_svr_sys_ptr;
    auto net_log_svr_ptr = std::make_shared<LogSvr>(log_svr_sys_ptr->IO(), LogSvrCfg());
    log_svr_sys_ptr->RegisterSvrFunc([net_log_svr_ptr] { net_log_svr_ptr->Start(); },
                                     [net_log_svr_ptr] { net_log_svr_ptr->Stop(); });

    t_svr_ = std::make_shared<std::thread>([log_svr_sys_ptr] {
      log_svr_sys_ptr->Start();
      log_svr_sys_ptr->Join();
      DBG_PRINT("log_svr_sys_ptr exit");
    });

    // log cli
    auto log_cli_sys_ptr = std::make_shared<AsioExecutor>(1);
    log_cli_sys_ptr_ = log_cli_sys_ptr;
    auto net_log_cli_ptr = std::make_shared<NetLogClient>(log_cli_sys_ptr->IO(), boost::asio::ip::tcp::endpoint{boost::asio::ip::address_v4({127, 0, 0, 1}), 50001});
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

    log_svr_sys_ptr_->Stop();
    t_svr_->join();

    log_cli_sys_ptr_->Stop();
    t_cli_->join();
  }

 private:
  std::shared_ptr<AsioExecutor> log_svr_sys_ptr_;
  std::shared_ptr<AsioExecutor> log_cli_sys_ptr_;

  std::shared_ptr<std::thread> t_svr_;
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
