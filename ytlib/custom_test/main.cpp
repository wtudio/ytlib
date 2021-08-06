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

#include "ytlib/boost_asio/net_util.hpp"
#include "ytlib/misc/error.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");

  uint32_t n = 2;
  boost::asio::io_context io(n);
  boost::asio::steady_timer t(io);
  boost::asio::steady_timer t2(io);

  boost::asio::co_spawn(
      io,
      [&t, &t2]() -> boost::asio::awaitable<void> {
        try {
          for (uint32_t ii = 0; ii < 10; ++ii) {
            t.expires_after(std::chrono::seconds(1));
            std::cerr << "thread " << std::this_thread::get_id() << " run 1.\n";
            co_await t.async_wait(boost::asio::use_awaitable);
            t2.expires_after(std::chrono::seconds(2));
          }
        } catch (const std::exception& e) {
          std::cerr << "log svr accept connection get exception:" << e.what() << '\n';
        }
        co_return;
      },
      boost::asio::detached);

  boost::asio::co_spawn(
      io,
      [&t2]() -> boost::asio::awaitable<void> {
        try {
          for (uint32_t ii = 0; ii < 10; ++ii) {
            t2.expires_after(std::chrono::seconds(2));
            std::cerr << "thread " << std::this_thread::get_id() << " run 2.\n";
            co_await t2.async_wait(boost::asio::use_awaitable);
          }
        } catch (const std::exception& e) {
          std::cerr << "log svr accept connection get exception:" << e.what() << '\n';
        }
        co_return;
      },
      boost::asio::detached);

  std::list<std::thread> threads_;

  for (int ii = 0; ii < n; ++ii) {
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

  DBG_PRINT("********************end test*******************");
  return 0;
}
