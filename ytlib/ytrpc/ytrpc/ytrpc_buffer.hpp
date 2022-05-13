#pragma once

#include <cstdlib>
#include <memory>
#include <vector>

#include <boost/asio/buffer.hpp>

#include <google/protobuf/io/zero_copy_stream.h>

namespace ytlib {
namespace ytrpc {

class BufferVec {
 public:
  BufferVec() noexcept {}

  ~BufferVec() noexcept {
    for (auto& buffer : buffer_vec_) {
      std::free(buffer.first);
    }
  }

  BufferVec(BufferVec&& v) noexcept {
    buffer_vec_.swap(v.buffer_vec_);
    v.buffer_vec_.clear();
  }

  BufferVec(const BufferVec&) = delete;             ///< no copy
  BufferVec& operator=(const BufferVec&) = delete;  ///< no copy

  BufferVec& Swap(BufferVec& v) {
    buffer_vec_.swap(v.buffer_vec_);
    return *this;
  }

  BufferVec& Merge(BufferVec& v) {
    buffer_vec_.insert(buffer_vec_.end(), v.buffer_vec_.begin(), v.buffer_vec_.end());
    v.buffer_vec_.clear();
    return *this;
  }

  const std::pair<void*, size_t>& NewBuffer(size_t buf_size) {
    return buffer_vec_.emplace_back(std::malloc(buf_size), buf_size);
  }

  const std::pair<void*, size_t>& CurBuffer() const noexcept {
    return *(buffer_vec_.rbegin());
  }

  /**
   * @brief 调小最后当前最后一个buf的大小
   * @note 必须保证当前已经申请过buf，必须保证buf_size小于当前最后一个buf的size
   * @param buf_size
   */
  void CommitLastBuf(size_t buf_size) noexcept {
    buffer_vec_.rbegin()->second = buf_size;
  }

  std::vector<boost::asio::const_buffer> GetAsioConstBufferVec() const {
    std::vector<boost::asio::const_buffer> asio_const_buffer_vec;
    asio_const_buffer_vec.reserve(buffer_vec_.size());
    for (const auto& buffer : buffer_vec_) {
      asio_const_buffer_vec.push_back(boost::asio::const_buffer(buffer.first, buffer.second));
    }
    return asio_const_buffer_vec;
  }

  const std::vector<std::pair<void*, size_t>>& Vec() const noexcept {
    return buffer_vec_;
  }

 private:
  std::vector<std::pair<void*, size_t>> buffer_vec_;
};

template <size_t block_size = 1024>
class BufferVecZeroCopyOutputStream : public ::google::protobuf::io::ZeroCopyOutputStream {
 public:
  explicit BufferVecZeroCopyOutputStream(BufferVec& buffer_vec)
      : buffer_vec_(buffer_vec) {}

  ~BufferVecZeroCopyOutputStream() {}

  bool Next(void** data, int* size) override {
    if (cur_buf_used_size_ == block_size) {
      *data = buffer_vec_.NewBuffer(block_size).first;
      byte_count_ += (*size = block_size);
    } else {
      *data = static_cast<char*>(buffer_vec_.CurBuffer().first) + cur_buf_used_size_;
      byte_count_ += (*size = block_size - cur_buf_used_size_);
      cur_buf_used_size_ = block_size;
    }
    return true;
  }

  void BackUp(int count) override {
    cur_buf_used_size_ -= count;
    byte_count_ -= count;
  }

  int64_t ByteCount() const override {
    return byte_count_;
  }

  size_t LastBufSize() const {
    return cur_buf_used_size_;
  }

  /**
   * @brief 为包头申请一段空间
   * @note 必须在创建完成后序列化pb结构之前调用，head_size必须小于block_size
   * @param head_size
   * @return void*
   */
  void* InitHead(size_t head_size) {
    byte_count_ = cur_buf_used_size_ = head_size;
    return buffer_vec_.NewBuffer(block_size).first;
  }

 private:
  BufferVec& buffer_vec_;
  size_t cur_buf_used_size_ = block_size;
  int64_t byte_count_ = 0;
};

class BufferVecZeroCopyInputStream : public ::google::protobuf::io::ZeroCopyInputStream {
 public:
  explicit BufferVecZeroCopyInputStream(BufferVec& buffer_vec)
      : buffer_vec_(buffer_vec) {}

  ~BufferVecZeroCopyInputStream() {}

  bool Next(const void** data, int* size) override {
    const auto& buffer_vec = buffer_vec_.Vec();
    if (cur_buf_index_ >= buffer_vec.size())
      return false;

    if (cur_buf_unused_size_ == 0) {
      const auto& cur_buffer = buffer_vec[cur_buf_index_++];
      *data = cur_buffer.first;
      byte_count_ += (*size = cur_buffer.second);
    } else {
      const auto& cur_buffer = buffer_vec[cur_buf_index_ - 1];
      *data = static_cast<char*>(cur_buffer.first) + (cur_buffer.second - cur_buf_unused_size_);
      byte_count_ += (*size = cur_buf_unused_size_);
      cur_buf_unused_size_ = 0;
    }

    return true;
  }

  void BackUp(int count) override {
    cur_buf_unused_size_ += count;
    byte_count_ -= count;
  }

  bool Skip(int count) override {
    byte_count_ += count;

    const auto& buffer_vec = buffer_vec_.Vec();
    const size_t buffer_vec_size = buffer_vec.size();

    for (;;) {
      if (cur_buf_unused_size_ > count) {
        cur_buf_unused_size_ -= count;
        count = 0;
        break;
      }

      count -= cur_buf_unused_size_;

      if (cur_buf_index_ >= buffer_vec_size) break;

      cur_buf_unused_size_ = buffer_vec[cur_buf_index_++].second;
    }

    return (count == 0);
  }

  int64_t ByteCount() const override {
    return byte_count_;
  }

 private:
  BufferVec& buffer_vec_;
  size_t cur_buf_unused_size_ = 0;
  size_t cur_buf_index_ = 0;
  int64_t byte_count_ = 0;
};

}  // namespace ytrpc
}  // namespace ytlib