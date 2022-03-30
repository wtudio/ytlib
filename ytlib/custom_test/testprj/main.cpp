/**
 * @file main.cpp
 * @brief 自定义测试
 * @note 自定义相关测试
 * @author WT
 * @date 2019-07-26
 */

#include <coroutine>
#include <cstdarg>
#include <functional>
#include <future>
#include <iostream>
#include <list>
#include <map>
#include <stack>
#include <thread>
#include <vector>

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "ytlib/boost_asio/asio_debug_tools.hpp"
#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std;
using namespace ytlib;

boost::asio::awaitable<void> TestTimerCo1(boost::asio::steady_timer& t1, boost::asio::steady_timer& t2) {
  ASIO_DEBUG_HANDLE(TestTimerCo1);
  try {
    t1.expires_after(std::chrono::seconds(1));
    std::cerr << "thread " << std::this_thread::get_id() << " run TestTimerCo1.\n";
    co_await t1.async_wait(boost::asio::use_awaitable);
    t2.expires_after(std::chrono::seconds(123));
  } catch (const std::exception& e) {
    std::cerr << "TestTimerCo1 get exception:" << e.what() << '\n';
  }
  co_return;
}

boost::asio::awaitable<void> TestTimerCo2(boost::asio::steady_timer& t1, boost::asio::steady_timer& t2) {
  ASIO_DEBUG_HANDLE(TestTimerCo2);
  try {
    t2.expires_after(std::chrono::seconds(2));
    std::cerr << "thread " << std::this_thread::get_id() << " run TestTimerCo2.\n";
    co_await t2.async_wait(boost::asio::use_awaitable);
  } catch (const std::exception& e) {
    std::cerr << "TestTimerCo2 get exception:" << e.what() << '\n';
  }
  co_return;
}

/**
 * @brief
 * boost::asio::steady_timer 在协程1中async_wait时，如果在其他线程/协程里重新设置了expires_after，则会触发在协程1里的异常
 *
 */
void Test1() {
  uint32_t n = 2;
  boost::asio::io_context io(n);
  boost::asio::steady_timer t1(io);
  boost::asio::steady_timer t2(io);

  boost::asio::co_spawn(io, TestTimerCo1(t1, t2), boost::asio::detached);
  boost::asio::co_spawn(io, TestTimerCo2(t1, t2), boost::asio::detached);

  std::list<std::thread> threads_;

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_.emplace(threads_.end(), [&io] {
      std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
      io.run();
      std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
    });
  }

  for (auto itr = threads_.begin(); itr != threads_.end();) {
    itr->join();
    threads_.erase(itr++);
  }
}

boost::asio::awaitable<uint32_t> TestCo1(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(TestCo1);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run TestCo1-a.\n";
    boost::asio::steady_timer t1(io);
    t1.expires_after(std::chrono::seconds(1));
    co_await t1.async_wait(boost::asio::use_awaitable);
    std::cerr << "thread " << std::this_thread::get_id() << " run TestCo1-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "TestCo1 get exception:" << e.what() << '\n';
  }
  co_return 111;
}

boost::asio::awaitable<void> TestCo2(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(TestCo2);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run TestCo2-a.\n";
    uint32_t co1_re = co_await boost::asio::co_spawn(io, TestCo1(io), boost::asio::use_awaitable);
    std::cerr << "co1_re " << co1_re << " \n";
    std::cerr << "thread " << std::this_thread::get_id() << " run TestCo2-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "TestCo2 get exception:" << e.what() << '\n';
  }
  co_return;
}

boost::asio::awaitable<void> TestCo3(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(TestCo3);

  try {
    std::cerr << "thread " << std::this_thread::get_id() << " run TestCo3-a.\n";
    boost::asio::co_spawn(io, TestCo2(io), boost::asio::detached);
    std::cerr << "thread " << std::this_thread::get_id() << " run TestCo3-b.\n";
  } catch (const std::exception& e) {
    std::cerr << "TestCo3 get exception:" << e.what() << '\n';
  }
  co_return;
}

/**
 * @brief
 * 1、co_spawn创建的协程是类似于dispach的运行方式，会先尽可能的执行下去，直到遇到co_await
 * 2、使用 co_await boost::asio::co_spawn(io, handle, boost::asio::use_awaitable) 来在协程内部等待另一个协程
 *
 */
void Test2() {
  uint32_t n = 2;
  boost::asio::io_context io(n);

  boost::asio::co_spawn(io, TestCo3(io), boost::asio::detached);

  // boost::asio::strand<boost::asio::io_context::executor_type> strand = boost::asio::make_strand(io);

  std::list<std::thread> threads_;

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_.emplace(threads_.end(), [&io] {
      std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
      io.run();
      std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
    });
  }

  for (auto itr = threads_.begin(); itr != threads_.end();) {
    itr->join();
    threads_.erase(itr++);
  }
}

boost::asio::awaitable<void> TestHttpCli(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(TestHttpCli);

  try {
    std::string host = "baidu.com";
    std::string port = "80";
    std::string target = "/";

    boost::asio::ip::tcp::resolver resolver(io);
    auto const results = co_await resolver.async_resolve(host, port, boost::asio::use_awaitable);

    boost::beast::tcp_stream stream(io);
    stream.expires_after(std::chrono::seconds(3));
    co_await stream.async_connect(results, boost::asio::use_awaitable);

    boost::beast::http::request<boost::beast::http::string_body> req{boost::beast::http::verb::get, target, 11};
    req.set(boost::beast::http::field::host, host);
    req.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    stream.expires_after(std::chrono::seconds(3));
    co_await boost::beast::http::async_write(stream, req, boost::asio::use_awaitable);

    boost::beast::flat_buffer buffer;
    boost::beast::http::response<boost::beast::http::dynamic_body> res;
    stream.expires_after(std::chrono::seconds(3));
    co_await boost::beast::http::async_read(stream, buffer, res, boost::asio::use_awaitable);

    std::cout << res << std::endl;

    stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both);

  } catch (const std::exception& e) {
    std::cerr << "TestHttpCli get exception:" << e.what() << '\n';
  }
  co_return;
}

boost::asio::awaitable<void> TestHttpSvr(boost::asio::io_context& io) {
  ASIO_DEBUG_HANDLE(TestHttpSvr);

  try {
  } catch (const std::exception& e) {
    std::cerr << "TestHttpSvr get exception:" << e.what() << '\n';
  }
  co_return;
}

void Test3() {
  uint32_t n = 2;
  boost::asio::io_context io(n);

  boost::asio::co_spawn(io, TestHttpSvr(io), boost::asio::detached);
  boost::asio::co_spawn(io, TestHttpCli(io), boost::asio::detached);

  std::list<std::thread> threads_;

  for (uint32_t ii = 0; ii < n; ++ii) {
    threads_.emplace(threads_.end(), [&io] {
      std::cerr << "thread " << std::this_thread::get_id() << " start.\n";
      io.run();
      std::cerr << "thread " << std::this_thread::get_id() << " exit.\n";
    });
  }

  for (auto itr = threads_.begin(); itr != threads_.end();) {
    itr->join();
    threads_.erase(itr++);
  }
}

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  // Test1();

  // Test2();

  Test3();

  DBG_PRINT("%s", AsioDebugTool::Ins().GetStatisticalResult().c_str());

  DBG_PRINT("********************end test*******************");
  return 0;
}
