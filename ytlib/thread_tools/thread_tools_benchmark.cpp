#include <benchmark/benchmark.h>

#include "block_queue.hpp"
#include "channel.hpp"

namespace ytlib {

using TestQueue = BlockQueue<uint32_t>;
using TestChannel = Channel<uint32_t>;

//测试Enqueue耗时
static void BM_BlockQueue_Enqueue(benchmark::State& state) {
  uint32_t obj_num = 1000;

  for (auto _ : state) {
    TestQueue qu;
    for (uint32_t ii = 0; ii < obj_num; ++ii)
      qu.Enqueue(ii);
  }
}
BENCHMARK(BM_BlockQueue_Enqueue);

//测试bool BlockDequeue(T &item)在单线程下耗时
static void BM_BlockQueue_BlockDequeue(benchmark::State& state) {
  uint32_t obj_num = 1000;

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    TestQueue qu;
    for (uint32_t ii = 0; ii < obj_num; ++ii) {
      qu.Enqueue(ii);
    }
    state.ResumeTiming();  // 恢复计时

    for (uint32_t ii = 0; ii < obj_num; ++ii) {
      uint32_t re;
      qu.BlockDequeue(re);
      ++re;
    }
  }
}
BENCHMARK(BM_BlockQueue_BlockDequeue);

//测试bool BlockDequeue(std::function<void(T &&)> f)在单线程下耗时
static void BM_BlockQueue_BlockDequeue_f(benchmark::State& state) {
  uint32_t obj_num = 1000;

  auto f = [](uint32_t&& input) {
    uint32_t re = input;
    ++re;
  };

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    TestQueue qu;
    for (uint32_t ii = 0; ii < obj_num; ++ii) {
      qu.Enqueue(ii);
    }
    state.ResumeTiming();  // 恢复计时

    for (uint32_t ii = 0; ii < obj_num; ++ii) {
      qu.BlockDequeue(f);
    }
  }
}
BENCHMARK(BM_BlockQueue_BlockDequeue_f);

//测试Channel性能
static void BM_Channel(benchmark::State& state) {
  auto ch_size = state.range(0);

  uint32_t obj_num = 1000;

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
    state.ResumeTiming();  // 恢复计时

    ch.StartProcess();
    ch.StopProcess();
  }
}
BENCHMARK(BM_Channel)->Threads(1)->RangeMultiplier(2)->Range(1, 4);

}  // namespace ytlib

BENCHMARK_MAIN();
