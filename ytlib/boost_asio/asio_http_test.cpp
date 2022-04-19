#include <gtest/gtest.h>

#include <sstream>

#include "asio_http_cli.hpp"
#include "asio_http_svr.hpp"
#include "asio_tools.hpp"

namespace ytlib {

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = boost::beast::http;

TEST(BOOST_ASIO_TEST, HTTP_base) {
  AsioDebugTool::Ins().Reset();

  auto cli_sys_ptr = std::make_shared<AsioExecutor>(1);
  auto svr1_sys_ptr = std::make_shared<AsioExecutor>(2);
  auto svr2_sys_ptr = std::make_shared<AsioExecutor>(2);

  // cli
  auto http_cli_ptr = std::make_shared<AsioHttpClient>(cli_sys_ptr->IO(), AsioHttpClient::Cfg{"127.0.0.1", "80"});
  cli_sys_ptr->RegisterSvrFunc(std::function<void()>(), [http_cli_ptr] { http_cli_ptr->Stop(); });

  auto http_send_recv = [http_cli_ptr](bool expect_exp = false) -> asio::awaitable<void> {
    ASIO_DEBUG_HANDLE(http_send_recv_co);
    bool exp_flag = false;
    try {
      http::request<http::string_body> req{http::verb::get, "/", 11};
      req.set(http::field::host, "127.0.0.1");
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

      std::stringstream ss;
      ss << req << std::endl;
      DBG_PRINT("req:\n%s", ss.str().c_str());

      auto rsp = co_await http_cli_ptr->HttpSendRecvCo(req);

      ss.str("");
      ss << rsp << std::endl;
      DBG_PRINT("rsp:\n%s", ss.str().c_str());

      // check rsp
      EXPECT_EQ(rsp.result_int(), 404);
      EXPECT_EQ(rsp.base().result_int(), 404);

      EXPECT_EQ(rsp.result(), http::status::not_found);
      EXPECT_EQ(rsp.base().result(), http::status::not_found);

      auto rsp_reason = rsp.reason();
      EXPECT_STREQ(std::string(rsp_reason.data(), rsp_reason.size()).c_str(), "Not Found");
      rsp_reason = rsp.base().reason();
      EXPECT_STREQ(std::string(rsp_reason.data(), rsp_reason.size()).c_str(), "Not Found");

      auto rsp_content_length = rsp.at(http::field::content_length);
      EXPECT_STREQ(std::string(rsp_content_length.data(), rsp_content_length.size()).c_str(), "31");
      rsp_content_length = rsp.base().at(http::field::content_length);
      EXPECT_STREQ(std::string(rsp_content_length.data(), rsp_content_length.size()).c_str(), "31");

      auto rsp_content_type = rsp.at(http::field::content_type);
      EXPECT_STREQ(std::string(rsp_content_type.data(), rsp_content_type.size()).c_str(), "text/html");
      rsp_content_type = rsp.base().at(http::field::content_type);
      EXPECT_STREQ(std::string(rsp_content_type.data(), rsp_content_type.size()).c_str(), "text/html");

      EXPECT_EQ(rsp.body().size(), 31);

      EXPECT_STREQ(rsp.body().c_str(), "The resource '/' was not found.");

    } catch (const std::exception& e) {
      DBG_PRINT("http_send_recv_co get exception and exit, exception: %s", e.what());
      exp_flag = true;
    }
    EXPECT_EQ(exp_flag, expect_exp);
    co_return;
  };

  std::thread t_cli([cli_sys_ptr] {
    DBG_PRINT("cli_sys_ptr start");
    cli_sys_ptr->Start();
    cli_sys_ptr->Join();
    DBG_PRINT("cli_sys_ptr exit");
  });

  // svr1
  std::thread t_svr1([svr1_sys_ptr] {
    DBG_PRINT("svr1_sys_ptr start");
    auto http_svr_ptr = std::make_shared<AsioHttpServer>(svr1_sys_ptr->IO(), AsioHttpServer::Cfg());
    svr1_sys_ptr->RegisterSvrFunc([http_svr_ptr] { http_svr_ptr->Start(); },
                                  [http_svr_ptr] { http_svr_ptr->Stop(); });

    svr1_sys_ptr->Start();
    svr1_sys_ptr->Join();
    DBG_PRINT("svr1_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(), asio::detached);
  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(), asio::detached);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(true), asio::detached);
  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(true), asio::detached);
  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(), asio::detached);
  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(), asio::detached);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  svr1_sys_ptr->Stop();
  t_svr1.join();

  // svr2
  std::thread t_svr2([svr2_sys_ptr] {
    DBG_PRINT("svr2_sys_ptr start");
    AsioHttpServer::Cfg cfg;
    auto http_svr_ptr = std::make_shared<AsioHttpServer>(svr2_sys_ptr->IO(), cfg);
    svr2_sys_ptr->RegisterSvrFunc([http_svr_ptr] { http_svr_ptr->Start(); },
                                  [http_svr_ptr] { http_svr_ptr->Stop(); });
    svr2_sys_ptr->Start();
    svr2_sys_ptr->Join();
    DBG_PRINT("svr2_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(true), asio::detached);
  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(true), asio::detached);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(), asio::detached);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  svr2_sys_ptr->Stop();
  t_svr2.join();

  cli_sys_ptr->Stop();
  t_cli.join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());
}

TEST(BOOST_ASIO_TEST, HTTP_handle) {
  AsioDebugTool::Ins().Reset();

  auto cli_sys_ptr = std::make_shared<AsioExecutor>(1);
  auto svr_sys_ptr = std::make_shared<AsioExecutor>(2);

  // cli
  auto http_cli_ptr = std::make_shared<AsioHttpClient>(cli_sys_ptr->IO(), AsioHttpClient::Cfg{"127.0.0.1", "80"});
  cli_sys_ptr->RegisterSvrFunc(std::function<void()>(), [http_cli_ptr] { http_cli_ptr->Stop(); });

  auto http_send_recv = [http_cli_ptr](bool expect_exp = false) -> asio::awaitable<void> {
    ASIO_DEBUG_HANDLE(http_send_recv_co);
    bool exp_flag = false;
    try {
      http::request<http::string_body> req{http::verb::post, "/test?key1=val1&key2=val2", 11};
      req.set(http::field::host, "127.0.0.1");
      req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
      req.body() = "test data.";
      req.prepare_payload();

      std::stringstream ss;
      ss << req << std::endl;
      DBG_PRINT("req:\n%s", ss.str().c_str());

      auto rsp = co_await http_cli_ptr->HttpSendRecvCo(req);

      ss.str("");
      ss << rsp << std::endl;
      DBG_PRINT("rsp:\n%s", ss.str().c_str());

      // check rsp
      EXPECT_EQ(rsp.result_int(), 200);
      EXPECT_EQ(rsp.result(), http::status::ok);

      auto rsp_reason = rsp.reason();
      EXPECT_STREQ(std::string(rsp_reason.data(), rsp_reason.size()).c_str(), "OK");

      auto rsp_content_length = rsp.at(http::field::content_length);
      EXPECT_STREQ(std::string(rsp_content_length.data(), rsp_content_length.size()).c_str(), "16");

      auto rsp_content_type = rsp.at(http::field::content_type);
      EXPECT_STREQ(std::string(rsp_content_type.data(), rsp_content_type.size()).c_str(), "text/html");

      EXPECT_EQ(rsp.body().size(), 16);
      EXPECT_STREQ(rsp.body().c_str(), "echo: test data.");

    } catch (const std::exception& e) {
      DBG_PRINT("http_send_recv_co get exception and exit, exception: %s", e.what());
      exp_flag = true;
    }
    EXPECT_EQ(exp_flag, expect_exp);
    co_return;
  };

  std::thread t_cli([cli_sys_ptr] {
    DBG_PRINT("cli_sys_ptr start");
    cli_sys_ptr->Start();
    cli_sys_ptr->Join();
    DBG_PRINT("cli_sys_ptr exit");
  });

  // svr
  auto http_svr_ptr = std::make_shared<AsioHttpServer>(svr_sys_ptr->IO(), AsioHttpServer::Cfg());
  svr_sys_ptr->RegisterSvrFunc([http_svr_ptr] { http_svr_ptr->Start(); },
                               [http_svr_ptr] { http_svr_ptr->Stop(); });

  std::function<boost::asio::awaitable<http::response<http::string_body>>(const http::request<http::dynamic_body>&)>
      HttpHandle = [](const http::request<http::dynamic_body>& req)
      -> boost::asio::awaitable<http::response<http::string_body>> {
    std::stringstream ss;
    ss << req << std::endl;
    DBG_PRINT("handle req:\n%s", ss.str().c_str());

    auto rsp = http::response<http::string_body>{http::status::ok, req.version()};
    rsp.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    rsp.set(http::field::content_type, "text/html");
    rsp.keep_alive(req.keep_alive());
    rsp.body() = "echo: " + boost::beast::buffers_to_string(req.body().data());
    rsp.prepare_payload();

    ss.str("");
    ss << rsp << std::endl;
    DBG_PRINT("handle rsp:\n%s", ss.str().c_str());

    co_return rsp;
  };
  http_svr_ptr->RegisterHttpHandleFunc<http::string_body>("/test", HttpHandle);

  std::thread t_svr([svr_sys_ptr] {
    DBG_PRINT("svr_sys_ptr start");
    svr_sys_ptr->Start();
    svr_sys_ptr->Join();
    DBG_PRINT("svr_sys_ptr exit");
  });
  std::this_thread::sleep_for(std::chrono::seconds(1));

  asio::co_spawn(http_cli_ptr->Strand(), http_send_recv(), asio::detached);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  svr_sys_ptr->Stop();
  t_svr.join();

  cli_sys_ptr->Stop();
  t_cli.join();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());
}

}  // namespace ytlib
