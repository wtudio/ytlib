#include <benchmark/benchmark.h>

// #include "alloc_bench.hpp"
// #include "pb_bench.hpp"
#include "map_bench.hpp"

int main(int argc, char** argv) {
  // PbBenchInit();
  MapBenchInit();

  ::benchmark::Initialize(&argc, argv);
  if (::benchmark::ReportUnrecognizedArguments(argc, argv)) return 1;
  ::benchmark::RunSpecifiedBenchmarks();
  ::benchmark::Shutdown();
  return 0;
}
