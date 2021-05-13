#include <benchmark/benchmark.h>

#include "guid.hpp"

#include <functional>
#include <iostream>
#include <string>

using ytlib::Guid;
using ytlib::GuidGener;
using ytlib::ObjGuidGener;

static void BM_GuidGener(benchmark::State& state) {
  // 生成mac值
  std::string mac = "testmac::abc::def";
  std::string svr_id = "testsvr";
  int thread_id = 123;
  uint32_t mac_hash = std::hash<std::string>{}(mac + svr_id + std::to_string(thread_id)) % ytlib::GUID_MAC_NUM;

  GuidGener::Ins().Init(mac_hash);

  // 生成obj值
  std::string obj_name = "test_obj_name";
  uint32_t obj_hash = std::hash<std::string>{}(obj_name) % ytlib::GUID_OBJ_NUM;

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
  uint32_t mac_hash = std::hash<std::string>{}(mac + svr_id + std::to_string(thread_id)) % ytlib::GUID_MAC_NUM;

  GuidGener::Ins().Init(mac_hash);

  // 生成obj值
  std::string obj_name = "test_obj_name";
  uint32_t obj_hash = std::hash<std::string>{}(obj_name) % ytlib::GUID_OBJ_NUM;
  ObjGuidGener gener;
  gener.Init(obj_hash);

  for (auto _ : state) {
    Guid guid = gener.GetGuid();
  }
}
BENCHMARK(BM_ObjGuidGener);

BENCHMARK_MAIN();