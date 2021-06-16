/**
 * @file main.cpp
 * @brief 自定义测试
 * @details 自定义相关测试
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

#include "ytlib/misc/error.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std;
using namespace ytlib;

std::list<std::thread> tlist;
std::mutex mu;

// 同步阻塞work
int SendRecv(int in) {
  std::this_thread::sleep_for(500ms);
  mu.lock();
  std::cout << "run in thread:" << std::this_thread::get_id()
            << ". in: " << in << std::endl;
  mu.unlock();
  return in;
}

// 异步回调work
using cbtype = std::function<void(int re)>;

void SendRecv_cb(int in, cbtype cb) {
  tlist.emplace(tlist.end(), [=] {
    cb(SendRecv(in));
  });
}

// future work
std::future<int> SendRecv_fu(int in) {
  return std::async([=] {
    return SendRecv(in);
  });
}

// 协程
struct CoReturnObj {
  struct promise_type {
    promise_type() {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", promise_type construct" << std::endl;
    }
    ~promise_type() {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", promise_type delet" << std::endl;
    }

    CoReturnObj get_return_object() {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", get_return_object" << std::endl;

      return CoReturnObj(this);
    }
    std::suspend_never initial_suspend() {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", initial_suspend" << std::endl;
      return {};
    }
    std::suspend_never final_suspend() noexcept {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", final_suspend" << std::endl;
      return {};
    }

    /*
    void return_void() {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", return_void" << std::endl;
    }
    */

    void return_value(int re) {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", return_value" << std::endl;
      p_ret->set_value(re);
    }

    std::suspend_never yield_value(int re) {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", yield_value" << std::endl;
      p_ret->set_value(re);
      return {};
    }

    void unhandled_exception() {
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ", unhandled_exception" << std::endl;
    }

    std::promise<int>* p_ret = nullptr;
  };

  CoReturnObj(promise_type* p) : handle(std::coroutine_handle<promise_type>::from_promise(*p)) {
    p->p_ret = &ret;
  }

  std::promise<int> ret;
  std::coroutine_handle<promise_type> handle;
};

template <class T>
struct Awaitable {
  Awaitable(std::function<void(std::function<void(T re)>)> anyscfun) {
    anyscfun_ = anyscfun;
  }

  constexpr bool await_ready() const noexcept {
    std::cout << "run in thread:" << std::this_thread::get_id()
              << ", await_ready" << std::endl;
    return false;
  }
  void await_suspend(std::coroutine_handle<> h) {
    std::cout << "run in thread:" << std::this_thread::get_id()
              << ", await_suspend" << std::endl;

    anyscfun_([=](T re) {
      this->re_ = re;
      h.resume();
    });
  }
  T await_resume() noexcept {
    std::cout << "run in thread:" << std::this_thread::get_id()
              << ", await_resume" << std::endl;
    return re_;
  }

  std::function<void(cbtype)> anyscfun_;
  T re_;
};

CoReturnObj SendRecv_co(int in) {
  int ret1 = co_await Awaitable<int>([in](std::function<void(int re)> cb) {
    SendRecv_cb(in, cb);
  });

  co_yield 100;

  int ret2 = co_await Awaitable<int>([in](std::function<void(int re)> cb) {
    SendRecv_cb(in + 1, cb);
  });
  co_return ret1 + ret2;
}

void fff(std::string&& s) {
  string s1;
  s1 = std::move(s);
}

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");
  //tcout输出中文需要设置
  //建议：最好不要在程序中使用中文！！！
  //std::locale::global(std::locale(""));
  //wcout.imbue(locale(""));

  std::string s1 = "abc";
  fff(s1);
  fff(std::move(s1));

  // 同步阻塞work
  {
    int re = SendRecv(1);
  }

  // 异步回调work
  {
    SendRecv_cb(2, [](int re) {
      mu.lock();
      std::cout << "run in thread:" << std::this_thread::get_id()
                << ". cb ret: " << re << std::endl;
      mu.unlock();
    });
  }

  // future work
  {
    std::future<int> re = SendRecv_fu(3);
    int reget = re.get();
    mu.lock();
    std::cout << "run in thread:" << std::this_thread::get_id()
              << ". fu ret: " << reget << std::endl;
    mu.unlock();
  }

  // 协程
  {
    auto re = SendRecv_co(4);
    auto fu = re.ret.get_future();
    int reget = fu.get();
    mu.lock();
    std::cout << "run in thread:" << std::this_thread::get_id()
              << ". co ret: " << reget << std::endl;
    mu.unlock();
  }

  for (auto itr = tlist.begin(); itr != tlist.end();) {
    itr->join();
    tlist.erase(itr++);
  }

  DBG_PRINT("********************end test*******************");
  return 0;
}
