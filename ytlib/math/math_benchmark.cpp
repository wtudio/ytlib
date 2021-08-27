#include <benchmark/benchmark.h>

#include <algorithm>
#include <vector>

#include "big_num.hpp"
#include "sort_algs.hpp"

namespace ytlib {

static void BM_BubbleSort(benchmark::State& state) {
  auto sort_size = state.range(0);
  std::vector<int> vec(static_cast<uint32_t>(sort_size));

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    std::generate(vec.begin(), vec.end(), [] { return rand(); });
    state.ResumeTiming();  // 恢复计时

    BubbleSort(vec.data(), vec.size());
  }
}
BENCHMARK(BM_BubbleSort)->RangeMultiplier(16)->Range(256, 65536);

static void BM_MergeSort(benchmark::State& state) {
  auto sort_size = state.range(0);
  std::vector<int> vec(static_cast<uint32_t>(sort_size));

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    std::generate(vec.begin(), vec.end(), [] { return rand(); });
    state.ResumeTiming();  // 恢复计时

    MergeSort(vec.data(), vec.size());
  }
}
BENCHMARK(BM_MergeSort)->RangeMultiplier(16)->Range(256, 65536);

static void BM_MergeSort2(benchmark::State& state) {
  auto sort_size = state.range(0);
  std::vector<int> vec(static_cast<uint32_t>(sort_size));

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    std::generate(vec.begin(), vec.end(), [] { return rand(); });
    state.ResumeTiming();  // 恢复计时

    MergeSort2(vec.data(), vec.size());
  }
}
BENCHMARK(BM_MergeSort2)->RangeMultiplier(16)->Range(256, 65536);

static void BM_QuickSort(benchmark::State& state) {
  auto sort_size = state.range(0);
  std::vector<int> vec(static_cast<uint32_t>(sort_size));

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    std::generate(vec.begin(), vec.end(), [] { return rand(); });
    state.ResumeTiming();  // 恢复计时

    QuickSort(vec.data(), vec.size());
  }
}
BENCHMARK(BM_QuickSort)->RangeMultiplier(16)->Range(256, 65536);

static void BM_StdSort(benchmark::State& state) {
  auto sort_size = state.range(0);
  std::vector<int> vec(static_cast<uint32_t>(sort_size));

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    std::generate(vec.begin(), vec.end(), [] { return rand(); });
    state.ResumeTiming();  // 恢复计时

    std::sort(vec.data(), vec.data() + vec.size());
  }
}
BENCHMARK(BM_StdSort)->RangeMultiplier(16)->Range(256, 65536);

// 大数工具测试
static void BM_BigNum(benchmark::State& state) {
  const uint32_t base = static_cast<uint32_t>(state.range(0));

  for (auto _ : state) {
    state.PauseTiming();  // 暂停计时
    BigNum num1("987654321987654321987654321987654321987654321987654321");
    num1.ReBase(base);
    BigNum num2("123456789123456789123456789");
    num2.ReBase(base);
    state.ResumeTiming();  // 恢复计时

    BigNum num3 = num1 / num2;
    BigNum num4 = num1 % num2;

    BigNum num5 = num3 * num2 + num4;
    bool flag = (num5 == num1);
    if (!flag) {
      state.SkipWithError("Failed!");
      break;
    }
  }
}
BENCHMARK(BM_BigNum)->Arg(2)->Arg(10)->Arg(10000)->Arg(UINT32_MAX);

}  // namespace ytlib

BENCHMARK_MAIN();
