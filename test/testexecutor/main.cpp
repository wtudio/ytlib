#include <iostream>
#include <thread>

#include <unifex/execute.hpp>
#include <unifex/scheduler_concepts.hpp>
#include <unifex/single_thread_context.hpp>

#include "ytlib/misc/misc_macro.h"
#include "ytlib/thread/thread_id.hpp"

class MyContext {
 public:
  explicit MyContext(uint32_t thread_num) : thread_([this] {}) {
  }
  ~MyContext() {}

 private:
  std::thread thread_;
};

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("[run in thread %llu]main.", ytlib::GetThreadId());

  unifex::single_thread_context ctx;

  for (int i = 0; i < 5; ++i) {
    unifex::execute(ctx.get_scheduler(), [i]() {
      DBG_PRINT("[run in thread %llu]hello execute().", ytlib::GetThreadId());
    });
  }

  return 0;
}
