/**
 * @file Serialize.h
 * @brief 基于boost的序列化
 * @details 基于boost的序列化与反序列化工具
 * @author WT
 * @date 2019-07-26
 */
#pragma once

#include "ytlib/misc/error.hpp"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/queue.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/shared_ptr_132.hpp>
#include <boost/serialization/shared_ptr_helper.hpp>
#include <boost/serialization/smart_cast.hpp>
#include <boost/serialization/stack.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

namespace ytlib {

///序列化类型
enum SerializeType {
  TextType,    //文本类型
  BinaryType,  //二进制类型
};

///侵入式序列化类头
#define T_CLASS_SERIALIZE(Members)           \
  friend class boost::serialization::access; \
  template <class Archive>                   \
  void serialize(Archive& ar, const uint32_t version) { ar Members; }

/**
 * @brief 序列化到字符串
 * @details 基于boost的序列化，仅适用于序列化规模较小，需要直接转化为string、不太考虑性能的场合
 * @param T 模板参数，待序列化的类型，需要在类开头添加侵入式序列化类头
 * @param obj 待序列化的实例
 * @param Type 序列化类型：文本型/二进制型
 * @return 存储序列化结果的字符串
 */
template <class T>
std::string Serialize(const T& obj, SerializeType Type = TextType) {
  std::ostringstream oss(std::ios_base::binary);
  if (Type == TextType) {
    boost::archive::text_oarchive oar(oss);
    oar << obj;
  } else {
    boost::archive::binary_oarchive oar(oss);
    oar << obj;
  }
  return oss.str();
}

/**
 * @brief 从字符串反序列化
 * @details 基于boost的反序列化，仅适用于序列化规模较小，需要直接转化为string、不太考虑性能的场合
 * @param T 模板参数，待反序列化的类型，需要在类开头添加侵入式序列化类头
 * @param obj 待被反序列化的实例
 * @param data 待反序列化的字符串
 * @param Type 反序列化类型：文本型/二进制型
 */
template <class T>
void Deserialize(T& obj, const std::string& data, SerializeType Type = TextType) {
  std::istringstream iss(data, std::ios_base::binary);
  if (Type == TextType) {
    boost::archive::text_iarchive iar(iss);
    iar >> obj;
  } else {
    boost::archive::binary_iarchive iar(iss);
    iar >> obj;
  }
}

/**
 * @brief 序列化到char array
 * @details 基于boost的序列化，高性能无拷贝
 * @param T 模板参数，待序列化的类型，需要在类开头添加侵入式序列化类头
 * @param obj 待序列化的实例
 * @param buf 存储序列化结果的char array
 * @param len 存储序列化结果的char array的长度。如果len太小会抛异常
 * @param Type 序列化类型：文本型/二进制型
 * @return 序列化后占用的容量
 */
template <class T>
uint32_t Serialize(const T& obj, char* buf, uint32_t len, SerializeType Type = TextType) {
  class ostreambuf : public std::basic_streambuf<char, std::char_traits<char> > {
   public:
    ostreambuf(char* buf, std::streamsize len) { setp(buf, buf + len); }
    uint32_t getlen() const { return pptr() - pbase(); }
  };

  ostreambuf obuf(buf, len);
  std::ostream oss(&obuf);
  if (Type == TextType) {
    boost::archive::text_oarchive oar(oss);
    oar << obj;
  } else {
    boost::archive::binary_oarchive oar(oss);
    oar << obj;
  }
  return obuf.getlen();
}

/**
 * @brief 从char array反序列化
 * @details 基于boost的反序列化，高性能无拷贝
 * @param T 模板参数，待反序列化的类型，需要在类开头添加侵入式序列化类头
 * @param obj 待被反序列化的实例
 * @param buf 待反序列化的char array
 * @param len 待反序列化的char array的长度
 * @param Type 反序列化类型：文本型/二进制型
 */
template <class T>
void Deserialize(T& obj, const char* buf, uint32_t len, SerializeType Type = TextType) {
  class istreambuf : public std::basic_streambuf<char, std::char_traits<char> > {
   public:
    istreambuf(char* buf, std::streamsize len) { setg(buf, buf, buf + len); }
  };

  istreambuf ibuf(const_cast<char*>(buf), len);
  std::istream iss(&ibuf);
  if (Type == TextType) {
    boost::archive::text_iarchive iar(iss);
    iar >> obj;
  } else {
    boost::archive::binary_iarchive iar(iss);
    iar >> obj;
  }
}

/**
 * @brief 序列化并存储到文件
 * @details 基于boost的序列化，仅适用于序列化规模较小，需要直接存储到文件、不太考虑性能的场合
 * @param T 模板参数，待序列化的类型，需要在类开头添加侵入式序列化类头
 * @param obj 待序列化的实例
 * @param filepath 存储序列化结果的文件路径
 * @param Type 序列化类型：文本型/二进制型
 */
template <class T>
void Serialize(const T& obj, const std::filesystem::path& filepath, SerializeType Type = TextType) {
  const auto& parent_path = filepath.parent_path();
  if (std::filesystem::status(parent_path).type() != std::filesystem::file_type::directory)
    std::filesystem::create_directories(parent_path);

  std::ofstream oss(filepath, std::ios::trunc | std::ios::binary);
  RT_ASSERT(oss, "open file failed.");

  if (Type == TextType) {
    boost::archive::text_oarchive oar(oss);
    oar << obj;
  } else {
    boost::archive::binary_oarchive oar(oss);
    oar << obj;
  }
  oss.close();
}

/**
 * @brief 从文件反序列化
 * @details 基于boost的反序列化，仅适用于序列化规模较小，需要直接存储到文件、不太考虑性能的场合
 * @param T 模板参数，待反序列化的类型，需要在类开头添加侵入式序列化类头
 * @param obj 待被反序列化的实例
 * @param filepath 存储待反序列化数据的文件路径
 * @param Type 反序列化类型：文本型/二进制型
 */
template <class T>
void Deserialize(T& obj, const std::filesystem::path& filepath, SerializeType Type = TextType) {
  std::ifstream iss(filepath, std::ios::binary);
  RT_ASSERT(iss, "open file failed.");

  if (Type == TextType) {
    boost::archive::text_iarchive iar(iss);
    iar >> obj;
  } else {
    boost::archive::binary_iarchive iar(iss);
    iar >> obj;
  }
  iss.close();
}

}  // namespace ytlib
