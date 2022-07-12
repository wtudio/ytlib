#include <benchmark/benchmark.h>

#include "Head.pb.h"
#include "ytrpc_zero_copy_stream.hpp"

#include "ytrpc_buffer.hpp"

inline ytlib::ytrpc::ReqHead g_req_head;
inline boost::asio::streambuf g_req_head_buf;
inline ytlib::ytrpc::BufferVec g_buffer_vec;

static void BM_PB_Serialize_AsioZeroCopyOutputStream(benchmark::State& state) {
  for (auto _ : state) {
    std::shared_ptr<boost::asio::streambuf> req_head_buf = std::make_shared<boost::asio::streambuf>();
    ytlib::ytrpc::AsioZeroCopyOutputStream os(*req_head_buf, 1024);
    if (!g_req_head.SerializeToZeroCopyStream(&os)) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_PB_Serialize_AsioZeroCopyOutputStream);

static void BM_PB_Serialize_ostream(benchmark::State& state) {
  for (auto _ : state) {
    std::shared_ptr<boost::asio::streambuf> req_head_buf = std::make_shared<boost::asio::streambuf>();
    std::ostream os(req_head_buf.get());
    if (!g_req_head.SerializeToOstream(&os)) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_PB_Serialize_ostream);

static void BM_PB_Serialize_BufferVecZeroCopyOutputStream(benchmark::State& state) {
  for (auto _ : state) {
    ytlib::ytrpc::BufferVec buffer_vec;
    ytlib::ytrpc::BufferVecZeroCopyOutputStream<1024> os(buffer_vec);
    if (!g_req_head.SerializeToZeroCopyStream(&os)) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
    buffer_vec.CommitLastBuf(os.LastBufSize());
  }
}
BENCHMARK(BM_PB_Serialize_BufferVecZeroCopyOutputStream);

static void BM_PB_Parse_ParseFromArray(benchmark::State& state) {
  for (auto _ : state) {
    ytlib::ytrpc::ReqHead req_head;
    if (!req_head.ParseFromArray(g_req_head_buf.data().data(), g_req_head_buf.size())) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_PB_Parse_ParseFromArray);

static void BM_PB_Parse_ParseFromIstream(benchmark::State& state) {
  for (auto _ : state) {
    ytlib::ytrpc::ReqHead req_head;
    std::istream is(&g_req_head_buf);
    if (!req_head.ParseFromIstream(&is)) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_PB_Parse_ParseFromIstream);

static void BM_PB_Parse_ParseFromZeroCopyStream(benchmark::State& state) {
  for (auto _ : state) {
    ytlib::ytrpc::ReqHead req_head;
    ytlib::ytrpc::BufferVecZeroCopyInputStream is(g_buffer_vec);
    if (!req_head.ParseFromZeroCopyStream(&is)) [[unlikely]]
      throw std::runtime_error("XXXXXXXXXX");
  }
}
BENCHMARK(BM_PB_Parse_ParseFromZeroCopyStream);

void PbBenchInit() {
  g_req_head.set_req_id(12345);
  g_req_head.set_func(R"str(
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
11111111111111111111111111111111111111
22222222222222222222222222222222222222
33333333333333333333333333333333333333
})str");
  g_req_head.set_ddl_ms(987654321);

  {
    std::ostream os(&g_req_head_buf);
    g_req_head.SerializeToOstream(&os);
  }

  {
    ytlib::ytrpc::BufferVecZeroCopyOutputStream<1024> os(g_buffer_vec);
    g_req_head.SerializeToZeroCopyStream(&os);
    g_buffer_vec.CommitLastBuf(os.LastBufSize());
  }
}