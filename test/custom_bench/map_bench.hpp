#include <benchmark/benchmark.h>

#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>

inline std::map<std::string, std::string> g_map;
inline std::map<size_t, std::string> g_map_hash;
inline std::unordered_map<std::string, std::string> g_umap;
inline std::unordered_map<size_t, std::string> g_umap_hash;

inline std::string g_to_find_key;

static void BM_MAP_FIND(benchmark::State& state) {
  for (auto _ : state) {
    auto finditr = g_map.find(g_to_find_key);
    if (finditr == g_map.end()) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_MAP_FIND);

static void BM_MAP_HASH_FIND(benchmark::State& state) {
  for (auto _ : state) {
    auto finditr = g_map_hash.find(std::hash<std::string>{}(g_to_find_key));
    if (finditr == g_map_hash.end()) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_MAP_HASH_FIND);

static void BM_UMAP_FIND(benchmark::State& state) {
  for (auto _ : state) {
    auto finditr = g_umap.find(g_to_find_key);
    if (finditr == g_umap.end()) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_UMAP_FIND);

static void BM_UMAP_HASH_FIND(benchmark::State& state) {
  for (auto _ : state) {
    auto finditr = g_umap_hash.find(std::hash<std::string>{}(g_to_find_key));
    if (finditr == g_umap_hash.end()) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_UMAP_HASH_FIND);

void MapBenchInit() {
  const size_t test_len = 4;
  std::string pre = "/testservice/testfunc";
  for (size_t ct = 0; ct < test_len; ++ct) {
    std::string key = pre + std::to_string(ct);
    std::string val = "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";

    size_t key_hash = std::hash<std::string>{}(key);

    g_map.emplace(key, val);
    g_map_hash.emplace(key_hash, val);
    g_umap.emplace(key, val);
    g_umap_hash.emplace(key_hash, val);
  }

  g_to_find_key = pre + "2";
}