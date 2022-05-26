#include <iostream>
#include <thread>

#include <boost/fiber/all.hpp>
#include <boost/fiber/detail/thread_barrier.hpp>

using namespace std;

/*
mutex 锁
condition 条件变量
barriers 同步
buffered/unbuffered channel 通道，无缓冲通道会在生产者端阻塞
future/promise
*/

void Test1() {
  std::cout << "[run in thread " << std::this_thread::get_id() << "] main\n";

  try {
    std::string str = "abc";
    int n = 5;

    boost::fibers::fiber f1([&str, &n]() {
      for (int i = 0; i < n; ++i) {
        std::cout << "[run in thread " << std::this_thread::get_id() << "]"
                  << i << ": " << str << std::endl;
        boost::this_fiber::yield();
      }
    });

    std::cerr << "f1 : " << f1.get_id() << std::endl;

    f1.join();

    std::cout << "done." << std::endl;

  } catch (std::exception const& e) {
    std::cerr << "exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "unhandled exception" << std::endl;
  }
}

void Test2() {
  try {
    int value1 = 0;
    int value2 = 0;

    boost::fibers::fiber f1([&value1]() {
      boost::fibers::fiber::id this_id = boost::this_fiber::get_id();
      for (int i = 0; i < 5; ++i) {
        ++value1;
        std::cout << "[run in thread " << std::this_thread::get_id() << "]"
                  << "fiber " << this_id << " fn1: increment value1: " << value1 << std::endl;
        boost::this_fiber::yield();
      }
      std::cout << "[run in thread " << std::this_thread::get_id() << "]"
                << "fiber " << this_id << " fn1: returns" << std::endl;
    });

    boost::fibers::fiber f2([&f1, &value2]() {
      boost::fibers::fiber::id this_id = boost::this_fiber::get_id();
      for (int i = 0; i < 5; ++i) {
        ++value2;
        std::cout << "[run in thread " << std::this_thread::get_id() << "]"
                  << "fiber " << this_id << " fn2: increment value2: " << value2 << std::endl;
        if (i == 1) {
          boost::fibers::fiber::id id = f1.get_id();
          std::cout << "[run in thread " << std::this_thread::get_id() << "]"
                    << "fiber " << this_id << " fn2: joins fiber " << id << std::endl;
          f1.join();
          std::cout << "[run in thread " << std::this_thread::get_id() << "]"
                    << "fiber " << this_id << " fn2: joined fiber " << id << std::endl;
        }
        boost::this_fiber::yield();
      }
      std::cout << "[run in thread " << std::this_thread::get_id() << "]"
                << "fiber " << this_id << " fn2: returns" << std::endl;
    });

    f2.join();

    std::cout << "done." << std::endl;

  } catch (std::exception const& e) {
    std::cerr << "exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "unhandled exception" << std::endl;
  }
}

void Test3() {
  try {
    int value1 = 0;
    int value2 = 0;

    boost::fibers::barrier fb(2);

    boost::fibers::fiber f1([&value1, &fb]() {
      boost::fibers::fiber::id id = boost::this_fiber::get_id();
      std::cout << "fiber " << id << ": fn1 entered" << std::endl;

      ++value1;
      std::cout << "fiber " << id << ": fn1 incremented value1: " << value1 << std::endl;
      boost::this_fiber::yield();

      std::cout << "fiber " << id << ": fn1 waits for barrier" << std::endl;
      fb.wait();
      std::cout << "fiber " << id << ": fn1 passed barrier" << std::endl;

      ++value1;
      std::cout << "fiber " << id << ": fn1 incremented value1: " << value1 << std::endl;
      boost::this_fiber::yield();

      ++value1;
      std::cout << "fiber " << id << ": fn1 incremented value1: " << value1 << std::endl;
      boost::this_fiber::yield();

      ++value1;
      std::cout << "fiber " << id << ": fn1 incremented value1: " << value1 << std::endl;
      boost::this_fiber::yield();

      std::cout << "fiber " << id << ": fn1 returns" << std::endl;
    });

    boost::fibers::fiber f2([&value2, &fb]() {
      boost::fibers::fiber::id id = boost::this_fiber::get_id();
      std::cout << "fiber " << id << ": fn2 entered" << std::endl;

      ++value2;
      std::cout << "fiber " << id << ": fn2 incremented value2: " << value2 << std::endl;
      boost::this_fiber::yield();

      ++value2;
      std::cout << "fiber " << id << ": fn2 incremented value2: " << value2 << std::endl;
      boost::this_fiber::yield();

      ++value2;
      std::cout << "fiber " << id << ": fn2 incremented value2: " << value2 << std::endl;
      boost::this_fiber::yield();

      std::cout << "fiber " << id << ": fn2 waits for barrier" << std::endl;
      fb.wait();
      std::cout << "fiber " << id << ": fn2 passed barrier" << std::endl;

      ++value2;
      std::cout << "fiber " << id << ": fn2 incremented value2: " << value2 << std::endl;
      boost::this_fiber::yield();

      std::cout << "fiber " << id << ": fn2 returns" << std::endl;
    });

    std::cout << "start f1 join." << std::endl;
    f1.join();

    std::cout << "start f2 join." << std::endl;
    f2.join();

    std::cout << "done." << std::endl;

  } catch (std::exception const& e) {
    std::cerr << "exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "unhandled exception" << std::endl;
  }
}

static std::size_t fiber_count{0};
static std::mutex mtx_count{};
static boost::fibers::condition_variable_any cnd_count{};
typedef std::unique_lock<std::mutex> lock_type;

void whatevah(char me) {
  try {
    std::thread::id my_thread = std::this_thread::get_id(); /*< get ID of initial thread >*/
    {
      std::ostringstream buffer;
      buffer << "fiber " << me << " started on thread " << my_thread << '\n';
      std::cout << buffer.str() << std::flush;
    }
    for (unsigned i = 0; i < 10; ++i) {                        /*< loop ten times >*/
      boost::this_fiber::yield();                              /*< yield to other fibers >*/
      std::thread::id new_thread = std::this_thread::get_id(); /*< get ID of current thread >*/
      if (new_thread != my_thread) {                           /*< test if fiber was migrated to another thread >*/
        my_thread = new_thread;
        std::ostringstream buffer;
        buffer << "fiber " << me << " switched to thread " << my_thread << '\n';
        std::cout << buffer.str() << std::flush;
      }
    }
  } catch (...) {
  }
  lock_type lk(mtx_count);
  if (0 == --fiber_count) { /*< Decrement fiber counter for each completed fiber. >*/
    lk.unlock();

    std::ostringstream buffer;
    buffer << "fiber " << me << " notify_all on thread " << std::this_thread::get_id() << '\n';
    std::cout << buffer.str() << std::flush;

    cnd_count.notify_all(); /*< Notify all fibers waiting on `cnd_count`. >*/
  }

  {
    std::ostringstream buffer;
    buffer << "fiber " << me << " end on thread " << std::this_thread::get_id() << '\n';
    std::cout << buffer.str() << std::flush;
  }
}

void ThreadFunc(boost::fibers::detail::thread_barrier* b) {
  std::ostringstream buffer;
  buffer << "thread started " << std::this_thread::get_id() << std::endl;
  std::cout << buffer.str() << std::flush;
  boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
  /*<
      Install the scheduling algorithm `boost::fibers::algo::shared_work` in order to
      join the work sharing.
  >*/
  b->wait(); /*< sync with other threads: allow them to start processing >*/
  lock_type lk(mtx_count);
  cnd_count.wait(lk, []() { return 0 == fiber_count; });
  /*<
      Suspend main fiber and resume worker fibers in the meanwhile.
      Main fiber gets resumed (e.g returns from `condition_variable_any::wait()`)
      if all worker fibers are complete.
  >*/
  // BOOST_ASSERT(0 == fiber_count);
}

void Test4() {
  std::cout << "main thread started " << std::this_thread::get_id() << std::endl;

  boost::fibers::use_scheduling_algorithm<boost::fibers::algo::shared_work>();
  /*<
      Install the scheduling algorithm `boost::fibers::algo::shared_work` in the main thread
      too, so each new fiber gets launched into the shared pool.
  >*/

  for (char c : std::string("abcdefghijklmnopqrstuvwxyz")) {
    /*<
      Launch a number of worker fibers; each worker fiber picks up a character
      that is passed as parameter to fiber-function `whatevah`.
      Each worker fiber gets detached.
    >*/
    boost::fibers::fiber([c]() { whatevah(c); }).detach();
    ++fiber_count; /*< Increment fiber counter for each new fiber. >*/
  }
  boost::fibers::detail::thread_barrier b(4);
  std::thread threads[] = {
      /*<
        Launch a couple of threads that join the work sharing.
      >*/
      std::thread(ThreadFunc, &b),
      std::thread(ThreadFunc, &b),
      std::thread(ThreadFunc, &b)};

  b.wait(); /*< sync with other threads: allow them to start processing >*/
  {
    lock_type lk(mtx_count);
    cnd_count.wait(lk, []() { return 0 == fiber_count; });
    /*<
        Suspend main fiber and resume worker fibers in the meanwhile.
        Main fiber gets resumed (e.g returns from `condition_variable_any::wait()`)
        if all worker fibers are complete.
    >*/
  }
  /*<
     Releasing lock of mtx_count is required before joining the threads, otherwise
     the other threads would be blocked inside condition_variable::wait() and
     would never return (deadlock).
  >*/
  // BOOST_ASSERT(0 == fiber_count);
  for (std::thread& t : threads) {
    /*< wait for threads to terminate >*/
    t.join();
  }

  std::cout << "done." << std::endl;
}

class AsyncAPI {
 public:
  // constructor acquires some resource that can be read and written
  AsyncAPI() : injected_(0) {}

  // callbacks accept an int error code; 0 == success
  typedef int errorcode;

  // write callback only needs to indicate success or failure
  template <typename Fn>
  void init_write(std::string const& data, Fn&& callback) {
    // make a local copy of injected_
    errorcode injected(injected_);
    // reset it synchronously with caller
    injected_ = 0;
    // update data_ (this might be an echo service)
    if (!injected) {
      data_ = data;
    }
    // Simulate an asynchronous I/O operation by launching a detached thread
    // that sleeps a bit before calling completion callback. Echo back to
    // caller any previously-injected errorcode.

    std::thread([injected, callback = std::forward<Fn>(callback)]() mutable {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      callback(injected);
    }).detach();
  }

  // read callback needs to accept both errorcode and data
  template <typename Fn>
  void init_read(Fn&& callback) {
    // make a local copy of injected_
    errorcode injected(injected_);
    // reset it synchronously with caller
    injected_ = 0;
    // local copy of data_ so we can capture in lambda
    std::string data(data_);
    // Simulate an asynchronous I/O operation by launching a detached thread
    // that sleeps a bit before calling completion callback. Echo back to
    // caller any previously-injected errorcode.

    std::thread([injected, callback = std::forward<Fn>(callback), data]() mutable {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      callback(injected, data);
    }).detach();
  }

  // ... other operations ...
  void inject_error(errorcode ec) { injected_ = ec; }

 private:
  std::string data_;
  errorcode injected_;
};

std::runtime_error make_exception(std::string const& desc, AsyncAPI::errorcode ec) {
  std::ostringstream buffer;
  buffer << "Error in AsyncAPI::" << desc << "(): " << ec;
  return std::runtime_error(buffer.str());
}

AsyncAPI::errorcode write_ec(AsyncAPI& api, std::string const& data) {
  boost::fibers::promise<AsyncAPI::errorcode> promise;
  boost::fibers::future<AsyncAPI::errorcode> future(promise.get_future());
  // In general, even though we block waiting for future::get() and therefore
  // won't destroy 'promise' until promise::set_value() has been called, we
  // are advised that with threads it's possible for ~promise() to be
  // entered before promise::set_value() has returned. While that shouldn't
  // happen with fibers::promise, a robust way to deal with the lifespan
  // issue is to bind 'promise' into our lambda. Since promise is move-only,
  // use initialization capture.

  api.init_write(
      data,
      [promise = std::move(promise)](AsyncAPI::errorcode ec) mutable {
        promise.set_value(ec);
      });

  return future.get();
}

void write(AsyncAPI& api, std::string const& data) {
  AsyncAPI::errorcode ec = write_ec(api, data);
  if (ec) {
    throw make_exception("write", ec);
  }
}

std::pair<AsyncAPI::errorcode, std::string> read_ec(AsyncAPI& api) {
  typedef std::pair<AsyncAPI::errorcode, std::string> result_pair;
  boost::fibers::promise<result_pair> promise;
  boost::fibers::future<result_pair> future(promise.get_future());
  // We promise that both 'promise' and 'future' will survive until our
  // lambda has been called.

  api.init_read([promise = std::move(promise)](AsyncAPI::errorcode ec, std::string const& data) mutable {
    promise.set_value(result_pair(ec, data));
  });

  return future.get();
}

std::string read(AsyncAPI& api) {
  boost::fibers::promise<std::string> promise;
  boost::fibers::future<std::string> future(promise.get_future());
  // Both 'promise' and 'future' will survive until our lambda has been
  // called.

  api.init_read([&promise](AsyncAPI::errorcode ec, std::string const& data) mutable {
    if (!ec) {
      promise.set_value(data);
    } else {
      promise.set_exception(
          std::make_exception_ptr(
              make_exception("read", ec)));
    }
  });

  return future.get();
}

void Test5() {
  AsyncAPI api;

  // successful write(): prime AsyncAPI with some data
  write(api, "abcd");
  // successful read(): retrieve it
  std::string data = read(api);
  std::cout << "read data " << data << std::endl;
  assert(data == "abcd");

  // successful write_ec()
  AsyncAPI::errorcode ec = write_ec(api, "efgh");
  assert(ec == 0);

  // write_ec() with error
  api.inject_error(1);
  ec = write_ec(api, "ijkl");
  assert(ec == 1);

  // write() with error
  std::string thrown;
  api.inject_error(2);
  try {
    write(api, "mnop");
  } catch (std::exception const& e) {
    thrown = e.what();
  }
  assert(thrown == make_exception("write", 2).what());

  std::tie(ec, data) = read_ec(api);
  assert(!ec);
  assert(data == "efgh");  // last successful write_ec()

  // read_ec() with error
  api.inject_error(3);
  std::tie(ec, data) = read_ec(api);
  assert(ec == 3);
  // 'data' in unspecified state, don't test

  // read() with error
  thrown.clear();
  api.inject_error(4);
  try {
    data = read(api);
  } catch (std::exception const& e) {
    thrown = e.what();
  }
  assert(thrown == make_exception("read", 4).what());

  std::cout << "done." << std::endl;
}

void Test6() {
  using channel_t = boost::fibers::buffered_channel<std::string>;
  try {
    channel_t chan1{2}, chan2{2};

    boost::fibers::fiber fping([&chan1, &chan2] {
      chan1.push("ping");
      std::cout << chan2.value_pop() << "\n";
      chan1.push("ping");
      std::cout << chan2.value_pop() << "\n";
      chan1.push("ping");
      std::cout << chan2.value_pop() << "\n";
    });
    boost::fibers::fiber fpong([&chan1, &chan2] {
      std::cout << chan1.value_pop() << "\n";
      chan2.push("pong");
      std::cout << chan1.value_pop() << "\n";
      chan2.push("pong");
      std::cout << chan1.value_pop() << "\n";
      chan2.push("pong");
    });

    fping.join();
    fpong.join();

    std::cout << "done." << std::endl;

  } catch (std::exception const& e) {
    std::cerr << "exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "unhandled exception" << std::endl;
  }
}

void Test7() {
  using channel_t = boost::fibers::unbuffered_channel<unsigned int>;
  try {
    channel_t chan;

    boost::fibers::fiber f1([&chan]() {
      chan.push(1);
      chan.push(1);
      chan.push(2);
      chan.push(3);
      chan.push(5);
      chan.push(8);
      chan.push(12);
      chan.close();
    });

    boost::fibers::fiber f2([&chan]() {
      for (unsigned int value : chan) {
        std::cout << value << " ";
      }
      std::cout << std::endl;
    });

    f1.join();
    f2.join();

    std::cout << "done." << std::endl;

  } catch (std::exception const& e) {
    std::cerr << "exception: " << e.what() << std::endl;
  } catch (...) {
    std::cerr << "unhandled exception" << std::endl;
  }
}

void Test8() {
}

int32_t main(int32_t argc, char** argv) {
  // Test1();

  // Test2();

  // Test3();

  // Test4();

  // Test5();

  // Test6();

  // Test7();

  Test8();

  return 0;
}
