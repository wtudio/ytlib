#include <benchmark/benchmark.h>

#include "serialize.hpp"

namespace ytlib {

class CBenchTest {
  T_CLASS_SERIALIZE(&name &age &m &v &dbl_arr)
 public:
  std::string name;
  uint32_t age;
  std::map<std::string, std::string> m;
  std::vector<uint32_t> v;
  double dbl_arr[5];
};

static void BM_SERIALIZE_STRING(benchmark::State &state) {
  static CBenchTest test_obj{
      .name = "test name",
      .age = 999,
      .m = {{"k1", "v1"}, {"k2", "v2"}},
      .v = {111, 222, 333},
      .dbl_arr = {111.1,
                  222.2,
                  333.3,
                  444.4,
                  555.5}};
  for (auto _ : state) {
    std::string re = Serialize(test_obj, SerializeType::BinaryType);
  }
}
BENCHMARK(BM_SERIALIZE_STRING);

static void BM_SERIALIZE_CHAR_ARR(benchmark::State &state) {
  static CBenchTest test_obj{
      .name = "test name",
      .age = 999,
      .m = {{"k1", "v1"}, {"k2", "v2"}},
      .v = {111, 222, 333},
      .dbl_arr = {111.1,
                  222.2,
                  333.3,
                  444.4,
                  555.5}};

  for (auto _ : state) {
    const uint32_t len = 1024;
    char buf[len];
    uint32_t slen = Serialize(test_obj, buf, len, SerializeType::BinaryType);
  }
}
BENCHMARK(BM_SERIALIZE_CHAR_ARR);

}  // namespace ytlib

BENCHMARK_MAIN();
