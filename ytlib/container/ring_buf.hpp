/**
 * @file ring_buf.hpp
 * @brief RingBuf
 * @note 环形缓冲队列
 * @author WT
 * @date 2021-05-16
 */
#pragma once

#include <cinttypes>
#include <cstring>

#include "ytlib/misc/error.hpp"

namespace ytlib {

template <class T, uint32_t BUF_SIZE>
class RingBuf {
 public:
  RingBuf() {}
  virtual ~RingBuf() {}

  bool Empty() const { return wpos_ == rpos_; }
  bool Full() const { return (wpos_ >= rpos_) ? (wpos_ - rpos_ == BUF_SIZE - 1) : (rpos_ - wpos_ == 1); }

  uint32_t Capacity() const { return BUF_SIZE - 1; }
  uint32_t Size() const { return (wpos_ >= rpos_) ? (wpos_ - rpos_) : (wpos_ + BUF_SIZE - rpos_); }
  uint32_t UnusedCapacity() const { return (wpos_ >= rpos_) ? (BUF_SIZE + rpos_ - wpos_ - 1) : (rpos_ - wpos_ - 1); }

  bool Push(const T& item) {
    if (Full()) [[unlikely]]
      return false;
    content_[wpos_] = item;
    if (++wpos_ < BUF_SIZE) return true;
    wpos_ = 0;
    return true;
  }

  bool Push(T&& item) {
    if (Full()) [[unlikely]]
      return false;
    content_[wpos_] = std::move(item);
    if (++wpos_ < BUF_SIZE) return true;
    wpos_ = 0;
    return true;
  }

  bool Pop() {
    if (Empty()) [[unlikely]]
      return false;
    if (++rpos_ < BUF_SIZE) return true;
    rpos_ = 0;
    return true;
  }

  T& Top() {
    RT_ASSERT(!Empty(), "Buf is empty.");
    return content_[rpos_];
  }

  T& Get(const uint32_t& pos) {
    RT_ASSERT(!Empty() && pos < Size(), "Pos is invalid.");
    const uint32_t& cur_rpos = rpos_ + pos;
    return content_[((cur_rpos < BUF_SIZE) ? cur_rpos : (cur_rpos - BUF_SIZE))];
  }

  bool PushArray(const T* buf, const uint32_t& len) {
    if (len > UnusedCapacity()) [[unlikely]]
      return false;
    if (wpos_ + len <= BUF_SIZE) {
      memcpy(content_ + wpos_, buf, len * sizeof(T));
      wpos_ += len;
    } else {
      const uint32_t& tmp_size = BUF_SIZE - wpos_;
      memcpy(content_ + wpos_, buf, tmp_size * sizeof(T));
      memcpy(content_, buf + tmp_size, len - tmp_size * sizeof(T));
      wpos_ = len - tmp_size;
    }
    return true;
  }

  bool PopArray(const uint32_t& len) {
    if (len > Size()) [[unlikely]]
      return false;
    rpos_ += len;
    if (rpos_ < BUF_SIZE) return true;
    rpos_ -= BUF_SIZE;
    return true;
  }

  bool TopArray(T*& buf, const uint32_t& len) {
    if (len > Size()) [[unlikely]]
      return false;
    if (rpos_ + len <= wpos_) {
      buf = content_ + rpos_;
    } else {
      if (buf == nullptr) [[unlikely]]
        return false;

      const uint32_t& tmp_size = BUF_SIZE - rpos_;
      memcpy(buf, content_ + rpos_, tmp_size * sizeof(T));
      memcpy(buf + tmp_size, content_, len - tmp_size * sizeof(T));
    }
    return true;
  }

  bool GetArray(const uint32_t& pos, T*& buf, const uint32_t& len) {
    if (len + pos > Size()) [[unlikely]]
      return false;

    uint32_t cur_rpos = rpos_ + pos;
    if (cur_rpos >= BUF_SIZE) cur_rpos -= BUF_SIZE;

    if (cur_rpos + len <= wpos_) {
      buf = content_ + cur_rpos;
    } else {
      if (buf == nullptr) [[unlikely]]
        return false;

      const uint32_t& tmp_size = BUF_SIZE - cur_rpos;
      memcpy(buf, content_ + cur_rpos, tmp_size * sizeof(T));
      memcpy(buf + tmp_size, content_, len - tmp_size * sizeof(T));
    }
    return true;
  }

  void Clear() { wpos_ = rpos_ = 0; }

 protected:
  T content_[BUF_SIZE];

  uint32_t wpos_ = 0;  // 写位置
  uint32_t rpos_ = 0;  // 读位置
};

}  // namespace ytlib
