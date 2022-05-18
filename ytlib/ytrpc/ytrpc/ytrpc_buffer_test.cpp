#include <gtest/gtest.h>

#include "Head.pb.h"
#include "ytlib/pb_tools/pb_tools.hpp"
#include "ytrpc_buffer.hpp"

namespace ytlib {
namespace ytrpc {

TEST(YTRPC_TEST, BufferVec) {
  BufferVec buffer_vec;

  const auto& new_buffer = buffer_vec.NewBuffer(10);
  EXPECT_EQ(new_buffer.second, 10);

  const auto& cur_buffer = buffer_vec.CurBuffer();

  EXPECT_EQ(&new_buffer, &cur_buffer);
}

TEST(YTRPC_TEST, BufferVecZeroCopyOutputStream) {
  BufferVec buffer_vec;
  BufferVecZeroCopyOutputStream os(buffer_vec);

  void* data;
  int size;
  EXPECT_TRUE(os.Next(&data, &size));
  EXPECT_EQ(size, 256);
  EXPECT_EQ(os.ByteCount(), 256);

  EXPECT_TRUE(os.Next(&data, &size));
  EXPECT_EQ(size, 512);
  EXPECT_EQ(os.ByteCount(), 768);

  os.BackUp(100);
  EXPECT_EQ(os.ByteCount(), 668);

  void* data_2;
  int size_2;
  EXPECT_TRUE(os.Next(&data_2, &size_2));
  EXPECT_EQ(static_cast<char*>(data_2), static_cast<char*>(data) + 412);
  EXPECT_EQ(size_2, 100);
  EXPECT_EQ(os.ByteCount(), 768);

  os.BackUp(10);
  EXPECT_EQ(os.ByteCount(), 758);

  EXPECT_TRUE(os.Next(&data_2, &size_2));
  EXPECT_EQ(static_cast<char*>(data_2), static_cast<char*>(data) + 502);
  EXPECT_EQ(size_2, 10);
  EXPECT_EQ(os.ByteCount(), 768);
}

TEST(YTRPC_TEST, BufferVecZeroCopyInputStream) {
  BufferVec buffer_vec;
  buffer_vec.NewBuffer(10);
  buffer_vec.NewBuffer(20);
  buffer_vec.NewBuffer(30);
  buffer_vec.NewBuffer(40);
  buffer_vec.NewBuffer(50);
  buffer_vec.NewBuffer(60);
  buffer_vec.NewBuffer(70);

  BufferVecZeroCopyInputStream is(buffer_vec);

  const void* data;
  int size;
  EXPECT_TRUE(is.Next(&data, &size));  // 10
  EXPECT_EQ(size, 10);
  EXPECT_EQ(is.ByteCount(), 10);

  EXPECT_TRUE(is.Next(&data, &size));  // 20
  EXPECT_EQ(size, 20);
  EXPECT_EQ(is.ByteCount(), 30);

  is.BackUp(5);
  EXPECT_EQ(is.ByteCount(), 25);

  const void* data_2;
  int size_2;
  EXPECT_TRUE(is.Next(&data_2, &size));  // 20
  EXPECT_EQ(static_cast<const char*>(data_2), static_cast<const char*>(data) + 15);
  EXPECT_EQ(size, 5);
  EXPECT_EQ(is.ByteCount(), 30);

  EXPECT_TRUE(is.Next(&data_2, &size_2));  // 30
  EXPECT_EQ(size_2, 30);
  EXPECT_EQ(is.ByteCount(), 60);

  EXPECT_TRUE(is.Skip(7));
  EXPECT_EQ(is.ByteCount(), 67);

  EXPECT_TRUE(is.Next(&data_2, &size_2));  // 40
  EXPECT_EQ(size_2, 33);
  EXPECT_EQ(is.ByteCount(), 100);

  EXPECT_TRUE(is.Skip(62));
  EXPECT_EQ(is.ByteCount(), 162);

  EXPECT_TRUE(is.Next(&data_2, &size_2));  // 60
  EXPECT_EQ(size_2, 48);
  EXPECT_EQ(is.ByteCount(), 210);

  EXPECT_FALSE(is.Skip(100));
}

TEST(YTRPC_TEST, BufferVec_MISC) {
  ytrpchead::ReqHead req_head;
  req_head.set_req_id(12345);
  req_head.set_func("/test.helloworld.gre/testfun");
  req_head.set_ddl_ms(987654321);

  BufferVec buffer_vec;

  BufferVecZeroCopyOutputStream os(buffer_vec);

  EXPECT_TRUE(req_head.SerializeToZeroCopyStream(&os));
  size_t req_head_size = os.ByteCount();

  buffer_vec.CommitLastBuf(os.LastBufSize());

  ytrpchead::ReqHead req_head_1;

  BufferVecZeroCopyInputStream is(buffer_vec);
  EXPECT_TRUE(req_head_1.ParseFromZeroCopyStream(&is));
  size_t req_head_1_size = is.ByteCount();

  EXPECT_EQ(req_head_size, req_head_1_size);

  EXPECT_STREQ(Pb2PrettyJson(req_head).c_str(), Pb2PrettyJson(req_head_1).c_str());
}

}  // namespace ytrpc
}  // namespace ytlib
