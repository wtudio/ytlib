#include <benchmark/benchmark.h>

#include "ring_buf.hpp"

namespace ytlib {

class TestClass {
 public:
  TestClass() {}
  TestClass(uint32_t id) : id_(id) {}

  uint32_t id_ = 0;
};

const uint32_t kBufSize = 1000;
using RingBufTest = RingBuf<TestClass, kBufSize>;

static void BM_RingBuf_Push(benchmark::State& state) {
  for (auto _ : state) {
    RingBufTest ring;
    for (uint32_t ii = 0; ii < kBufSize; ++ii) {
      ring.Push(TestClass(ii));
    }
  }
}
BENCHMARK(BM_RingBuf_Push);

}  // namespace ytlib

BENCHMARK_MAIN();
