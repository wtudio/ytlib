#pragma once

#include <unifex/async_trace.hpp>

#include <unifex/scheduler_concepts.hpp>
#include <unifex/sequence.hpp>
#include <unifex/sync_wait.hpp>

#include <unifex/config.hpp>
#include <unifex/defer.hpp>
#include <unifex/finally.hpp>
#include <unifex/just.hpp>
#include <unifex/then.hpp>
#include <unifex/timed_single_thread_context.hpp>
#include <unifex/when_all.hpp>

#include <unifex/task.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

namespace test6 {

using namespace unifex;
using namespace std::chrono;
using namespace std::chrono_literals;

auto dump_async_trace(std::string tag = {}) {
  return then(
      async_trace_sender{},
      [tag = std::move(tag)](const std::vector<async_trace_entry> &entries) {
        std::cout << "Async Trace (" << tag << "):\n";
        for (auto &entry : entries) {
          std::cout << " " << entry.depth << " [-> " << entry.parentIndex
                    << "]: " << entry.continuation.type().name() << " @ 0x";
          std::cout.setf(std::ios::hex, std::ios::basefield);
          std::cout << entry.continuation.address();
          std::cout.unsetf(std::ios::hex);
          std::cout << "\n";
        }
      });
}

task<int> MyTask1(int a, int b) {
  co_return a + b;
}

task<int> MyTask2() {
  auto ret = co_await unifex::sequence(dump_async_trace("MyTask2"), MyTask1(1, 2));

  co_return ret;
}

task<void> MyTask3() {
  auto ret = co_await unifex::sequence(dump_async_trace("MyTask3"), MyTask2());

  co_return;
}

inline void Test6() {
  sync_wait(MyTask3());
}

}  // namespace test6
