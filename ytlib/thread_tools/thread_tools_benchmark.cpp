#include <benchmark/benchmark.h>

#include "block_queue.hpp"
#include "channel.hpp"

using TestQueue = ytlib::BlockQueue<uint32_t>;
using TestChannel = ytlib::Channel<uint32_t>;

//测试Enqueue在不同线程数下的耗时
static void BM_BlockQueue_Enqueue(benchmark::State& state) {
  auto th_size = state.range(0);

  uint32_t obj_num = 4096;
  uint32_t obj_num_per_th = obj_num / th_size;

  for (auto _ : state) {
    TestQueue qu;
    std::list<std::thread> threads;

    for (uint32_t ii = 0; ii < th_size; ++ii) {
      threads.emplace(threads.end(), [&] {
        for (uint32_t jj = 0; jj < obj_num_per_th; ++jj)
          qu.Enqueue(jj);
      });
    }
    for (auto itr = threads.begin(); itr != threads.end(); itr++) {
      itr->join();
    }
  }
}
BENCHMARK(BM_BlockQueue_Enqueue)->RangeMultiplier(4)->Range(1, 16);

//测试bool BlockDequeue(T &item)在单线程下耗时
static void BM_BlockQueue_BlockDequeue(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    TestQueue qu;
    for (uint32_t ii = 0; ii < 1000; ++ii) {
      qu.Enqueue(ii);
    }
    state.ResumeTiming();  // 恢复计时
    for (uint32_t ii = 0; ii < 1000; ++ii) {
      uint32_t re;
      qu.BlockDequeue(re);
      ++re;
    }
  }
}
BENCHMARK(BM_BlockQueue_BlockDequeue);

//测试bool BlockDequeue(std::function<void(T &&)> f)在单线程下耗时
static void BM_BlockQueue_BlockDequeue_f(benchmark::State& state) {
  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    TestQueue qu;
    for (uint32_t ii = 0; ii < 1000; ++ii) {
      qu.Enqueue(ii);
    }
    auto f = [&](uint32_t&& input) {
      uint32_t re = input;
      ++re;
    };
    state.ResumeTiming();  // 恢复计时
    for (uint32_t ii = 0; ii < 1000; ++ii) {
      qu.BlockDequeue(f);
    }
  }
}
BENCHMARK(BM_BlockQueue_BlockDequeue_f);

//测试异步Enqueue&Dequeue性能
static void BM_BlockQueue_Thread(benchmark::State& state) {
  auto th_size = state.range(0);

  uint32_t obj_num = 4096;
  uint32_t obj_num_per_th = obj_num / th_size;

  for (auto _ : state) {
    TestQueue qu;
    std::list<std::thread> threads;

    for (uint32_t ii = 0; ii < th_size; ++ii) {
      threads.emplace(threads.end(), [&] {
        for (uint32_t jj = 0; jj < obj_num_per_th; ++jj)
          qu.Enqueue(jj);
      });
    }

    std::thread t_c([&] {
      for (uint32_t ii = 0; ii < 1000; ++ii) {
        uint32_t re;
        qu.BlockDequeue(re);
        ++re;
      }
    });

    for (auto itr = threads.begin(); itr != threads.end(); itr++) {
      itr->join();
    }
    t_c.join();
  }
}
BENCHMARK(BM_BlockQueue_Thread)->RangeMultiplier(4)->Range(1, 16);

//测试异步Enqueue&Dequeue性能
static void BM_BlockQueue_Thread_f(benchmark::State& state) {
  auto th_size = state.range(0);

  uint32_t obj_num = 4096;
  uint32_t obj_num_per_th = obj_num / th_size;

  auto f = [](uint32_t&& input) {
    uint32_t re = input;
    ++re;
  };

  for (auto _ : state) {
    TestQueue qu;
    std::list<std::thread> threads;

    for (uint32_t ii = 0; ii < th_size; ++ii) {
      threads.emplace(threads.end(), [&] {
        for (uint32_t jj = 0; jj < obj_num_per_th; ++jj)
          qu.Enqueue(jj);
      });
    }

    std::thread t_c([&] {
      for (uint32_t ii = 0; ii < 1000; ++ii) {
        qu.BlockDequeue(f);
      }
    });

    for (auto itr = threads.begin(); itr != threads.end(); itr++) {
      itr->join();
    }
    t_c.join();
  }
}
BENCHMARK(BM_BlockQueue_Thread_f)->RangeMultiplier(4)->Range(1, 16);

//测试Channel性能
static void BM_Channel(benchmark::State& state) {
  auto ch_size = state.range(0);

  uint32_t obj_num = 128;

  auto f = [](uint32_t&& input) {
    uint32_t re = input;
    ++re;
  };
  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    TestChannel ch;
    ch.Init(f, ch_size);
    for (uint32_t jj = 0; jj < obj_num; ++jj)
      ch.Enqueue(jj);
    assert(ch.Count() == obj_num);
    state.ResumeTiming();  // 恢复计时
    ch.StartProcess();
    ch.StopProcess();
    assert(ch.Count() == 0);
  }
}
BENCHMARK(BM_Channel)->Threads(1)->RangeMultiplier(4)->Range(1, 16);

BENCHMARK_MAIN();