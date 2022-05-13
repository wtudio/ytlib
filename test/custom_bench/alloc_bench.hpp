#include <benchmark/benchmark.h>

#include <cstdlib>

inline std::allocator<char> alloc;

static void BM_STD_MALLOC(benchmark::State& state) {
  for (auto _ : state) {
    void* buf = std::malloc(1024);
    static_cast<char*>(buf)[0] = '0';
    std::free(buf);
  }
}
BENCHMARK(BM_STD_MALLOC);

static void BM_alloc(benchmark::State& state) {
  for (auto _ : state) {
    char* buf = alloc.allocate(1024);  // space for one int
    buf[0] = '0';
    alloc.deallocate(buf, 1024);  // and it is gone
  }
}
BENCHMARK(BM_alloc);

static void BM_malloc(benchmark::State& state) {
  for (auto _ : state) {
    void* buf = malloc(1024);
    static_cast<char*>(buf)[0] = '0';
    free(buf);
  }
}
BENCHMARK(BM_malloc);

static void BM_vector(benchmark::State& state) {
  for (auto _ : state) {
    std::vector<char> buf(1024);
    buf[0] = '0';
  }
}
BENCHMARK(BM_vector);
