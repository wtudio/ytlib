#pragma once

#include <google/protobuf/io/zero_copy_stream.h>
#include <boost/asio/streambuf.hpp>

namespace ytlib {
namespace ytrpc {

class AsioZeroCopyOutputStream : public ::google::protobuf::io::ZeroCopyOutputStream {
 public:
  explicit AsioZeroCopyOutputStream(boost::asio::streambuf& streambuf, size_t block_size = 1024)
      : block_size_(block_size),
        streambuf_(streambuf),
        buf_commit_size_(block_size) {}

  ~AsioZeroCopyOutputStream() {}

  bool Next(void** data, int* size) override {
    if (to_commit_size_ == 0) {
      if (buf_commit_size_ == block_size_) {
        buf_ = streambuf_.prepare(*size = to_commit_size_ = block_size_);
        *data = buf_.data();
        buf_commit_size_ = 0;
      } else {
        *data = static_cast<char*>(buf_.data()) + buf_commit_size_;
        *size = to_commit_size_ = block_size_ - buf_commit_size_;
      }
    } else {
      streambuf_.commit(to_commit_size_);
      byte_count_ += to_commit_size_;
      buf_ = streambuf_.prepare(*size = to_commit_size_ = block_size_);
      *data = buf_.data();
      buf_commit_size_ = 0;
    }

    return true;
  }

  /**
   * @brief 返还buf空间
   * @note 不会连续调用，在调用它之前一定会先调用Next，结束时一定会调用，哪怕count为0
   * @param count
   */
  void BackUp(int count) override {
    streambuf_.commit(to_commit_size_ -= count);
    byte_count_ += to_commit_size_;
    buf_commit_size_ += to_commit_size_;
    to_commit_size_ = 0;
  }

  google::protobuf::int64 ByteCount() const override {
    return byte_count_;
  }

  /**
   * @brief 为包头申请一段空间
   * @note 必须在创建完成后序列化pb结构之前调用，head_size必须小于block_size
   * @param head_size
   * @return void*
   */
  void* InitHead(size_t head_size) {
    buf_ = streambuf_.prepare(block_size_);
    streambuf_.commit(byte_count_ = buf_commit_size_ = head_size);
    return buf_.data();
  }

 private:
  const size_t block_size_;
  boost::asio::streambuf& streambuf_;
  boost::asio::mutable_buffer buf_;
  google::protobuf::int64 byte_count_ = 0;
  size_t to_commit_size_ = 0;
  size_t buf_commit_size_;
};

}  // namespace ytrpc
}  // namespace ytlib
