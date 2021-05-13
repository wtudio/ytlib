/**
 * @file main.cpp
 * @brief 测试所有接口
 * @details 单元测试所有接口
 * @author WT
 * @email 905976782@qq.com
 * @date 2019-07-26
 */
#include <ytlib/Common/Util.h>

#include "t_Algorithm.h"
#include "t_DataStructure.h"
#include "t_FileManager.h"
#include "t_LightMath.h"
#include "t_LogService.h"
#include "t_NetTools.h"
#include "t_ProcessManager.h"
#include "t_SupportTools.h"

#include <boost/core/lightweight_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <cstdarg>
#include <map>
#include <stack>
#include <vector>

#include "ytlib/StringUtil/StringUtil.h"
#include "ytlib/log/log.hpp"

using namespace std;
using namespace ytlib;

//测试运行时间小工具
boost::posix_time::ptime ticTime_global;
void setTime() { ticTime_global = boost::posix_time::microsec_clock::universal_time(); }
string getTime() {
  boost::posix_time::ptime tocTime_global = boost::posix_time::microsec_clock::universal_time();
  return to_string((tocTime_global - ticTime_global).ticks()) + "us";
}

int32_t main(int32_t argc, char** argv) {
  YT_DEBUG_PRINTF("-------------------start test-------------------");
  //tcout输出中文需要设置
  //建议：最好不要在程序中使用中文！！！
  //std::locale::global(std::locale(""));
  //wcout.imbue(locale(""));

  bool re = CheckIfInList("123,456,789", "123");

  //单元测试区
  //Algorithm
  test_SortAlgs();
  test_StringAlgs();

  //DataStructure
  test_BinaryTree();
  test_Graph();
  test_Heap();

  //网络测试区

  //其他测试区

  /*
  test_bignum();

  test_Serialize();
  test_stl();
  test_urlencode();
  test_LoopTool();
  test_NetLog();
  test_TcpNetAdapter();

  test_Complex();
  test_Matrix();
  test_Matrix_c();
  test_tools();

  test_KeyValueFile();
  test_SerializeFile();
  test_XMLFile();
  test_PrjBase();

  test_QueueProcess();
*/
  YT_DEBUG_PRINTF("********************end test*******************");
  return boost::report_errors();
}
