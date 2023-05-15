#include <benchmark/benchmark.h>

#include "block_queue.hpp"
#include "channel.hpp"
#include "guid.hpp"

namespace ytlib {

using TestQueue = BlockQueue<uint32_t>;
using TestChannel = Channel<uint32_t>;

// 测试Enqueue耗时
static void BM_BlockQueue_Enqueue(benchmark::State& state) {
  uint32_t obj_num = 1000;

  for (auto _ : state) {
    TestQueue qu;
    for (uint32_t ii = 0; ii < obj_num; ++ii)
      qu.Enqueue(ii);
  }
}
BENCHMARK(BM_BlockQueue_Enqueue);

// 测试bool BlockDequeue(T &item)在单线程下耗时
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

// 测试bool BlockDequeue(std::function<void(T &&)> f)在单线程下耗时
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

// 测试Channel性能
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
    ch.Init(f, static_cast<uint32_t>(ch_size));
    for (uint32_t jj = 0; jj < obj_num; ++jj)
      ch.Enqueue(jj);
    state.ResumeTiming();  // 恢复计时

    ch.StartProcess();
    ch.StopProcess();
  }
}
BENCHMARK(BM_Channel)->Threads(1)->RangeMultiplier(2)->Range(1, 4);

static void BM_GuidGener(benchmark::State& state) {
  // 生成mac值
  std::string mac = "testmac::abc::def";
  std::string svr_id = "testsvr";
  int thread_id = 123;
  uint32_t mac_hash = std::hash<std::string>{}(mac + svr_id + std::to_string(thread_id)) % GUID_MAC_NUM;

  GuidGener::Ins().Init(mac_hash);

  // 生成obj值
  std::string obj_name = "test_obj_name";
  uint32_t obj_hash = std::hash<std::string>{}(obj_name) % GUID_OBJ_NUM;

  for (auto _ : state) {
    Guid guid = GuidGener::Ins().GetGuid(obj_hash);
  }
}
BENCHMARK(BM_GuidGener);

static void BM_ObjGuidGener(benchmark::State& state) {
  // 生成mac值
  std::string mac = "testmac::abc::def";
  std::string svr_id = "testsvr";
  int thread_id = 123;
  uint32_t mac_hash = std::hash<std::string>{}(mac + svr_id + std::to_string(thread_id)) % GUID_MAC_NUM;

  GuidGener::Ins().Init(mac_hash);

  // 生成obj值
  std::string obj_name = "test_obj_name";
  uint32_t obj_hash = std::hash<std::string>{}(obj_name) % GUID_OBJ_NUM;
  ObjGuidGener gener;
  gener.Init(obj_hash);

  for (auto _ : state) {
    Guid guid = gener.GetGuid();
  }
}
BENCHMARK(BM_ObjGuidGener);

}  // namespace ytlib

BENCHMARK_MAIN();
