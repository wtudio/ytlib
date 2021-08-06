/**
 * @file make_final.hpp
 * @brief 无法被继承的类
 * @note 制作一个无法被继承的类
 * @author WT
 * @date 2019-07-26
 */
#pragma once

namespace ytlib {

/**
 * @brief 用来制作一个无法被继承的类
 *   class FinalClass2 : virtual public MakeFinal<FinalClass2>{
 *  public :
 *    FinalClass2() {}
 *    ~FinalClass2() {}
 *  };
 */
template <typename T>
class MakeFinal {
  friend T;

 private:
  MakeFinal() {}
  ~MakeFinal() {}
};

}  // namespace ytlib
