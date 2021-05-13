#include <benchmark/benchmark.h>

#include "log.hpp"

using ytlib::Log;

static void BM_StringCreation(benchmark::State& state) {
  for (auto _ : state) {
    Log::Ins().SetLevel(ytlib::LOG_LEVEL::L_INFO);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_StringCreation);

// Define another benchmark
static void BM_StringCopy(benchmark::State& state) {
  std::string x = "hello";
  for (auto _ : state)
    auto l = Log::Ins().Level();
}
BENCHMARK(BM_StringCopy);

BENCHMARK_MAIN();