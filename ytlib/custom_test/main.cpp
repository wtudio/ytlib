/**
 * @file main.cpp
 * @brief 自定义测试
 * @details 自定义相关测试
 * @author WT
 * @date 2019-07-26
 */

#include <cstdarg>
#include <map>
#include <stack>
#include <vector>

#include "ytlib/misc/error.hpp"
#include "ytlib/misc/misc_macro.h"

using namespace std;
using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
  DBG_PRINT("-------------------start test-------------------");
  //tcout输出中文需要设置
  //建议：最好不要在程序中使用中文！！！
  //std::locale::global(std::locale(""));
  //wcout.imbue(locale(""));

  DBG_PRINT("********************end test*******************");
  return 0;
}
