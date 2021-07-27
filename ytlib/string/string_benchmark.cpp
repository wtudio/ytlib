#include <benchmark/benchmark.h>

#include "string_algs.hpp"
#include "string_util.hpp"
#include "url_encode.hpp"

namespace ytlib {

static void BM_Demo(benchmark::State& state) {
  for (auto _ : state) {
    int i = 0;
  }
}
BENCHMARK(BM_Demo);

}  // namespace ytlib

BENCHMARK_MAIN();
