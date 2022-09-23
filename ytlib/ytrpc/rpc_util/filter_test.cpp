#include <gtest/gtest.h>

#include <string>

#include <unifex/sync_wait.hpp>

#include "filter.hpp"

namespace ytlib {
namespace ytrpc {

TEST(RPC_UTIL_TEST, FilterMgr) {
  using TestFilterMgr = FilterMgr<std::string, int, std::string, std::string>;
  TestFilterMgr mgr;

  std::string step;

  mgr.RegisterFilter([&step](const std::string& ctx, const std::string& req, std::string& rsp,
                             const TestFilterMgr::RpcHandle& next) -> unifex::task<int> {
    step += "1";
    auto st = co_await next(ctx, req, rsp);
    step += "2";
    co_return st;
  });

  mgr.RegisterFilter([&step](const std::string& ctx, const std::string& req, std::string& rsp,
                             const TestFilterMgr::RpcHandle& next) -> unifex::task<int> {
    step += "3";
    auto st = co_await next(ctx, req, rsp);
    step += "4";
    co_return st;
  });

  TestFilterMgr::RpcHandle rpc = [&step](const std::string& ctx, const std::string& req, std::string& rsp) -> unifex::task<int> {
    step += "5";
    rsp = "echo " + req;
    co_return 123;
  };

  std::string req = "testreq";
  std::string rsp;
  std::string ctx;

  auto ret = unifex::sync_wait(mgr.InvokeRpc(rpc, ctx, req, rsp));

  ASSERT_TRUE(ret);
  EXPECT_EQ(*ret, 123);
  EXPECT_STREQ(rsp.c_str(), "echo testreq");
  EXPECT_STREQ(step.c_str(), "31524");
}

}  // namespace ytrpc
}  // namespace ytlib