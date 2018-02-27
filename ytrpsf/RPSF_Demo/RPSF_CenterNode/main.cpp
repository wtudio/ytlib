#include <ytrpsf/CenterNode.h>

using namespace ytlib;
using namespace rpsf;
using namespace std;

//最简单的命令行中心节点
int32_t main(int32_t argc, char** argv) {
	tcout << T_TEXT("---------RPSF Center Node---------") << endl;
	//获取配置文件路径
	string cfgpath;
	if (argc == 1) {
		cfgpath = (tGetCurrentPath() / T_TEXT("CenterNode.xcfg")).string<string>();
	}
	else if (argc == 2) {
		cfgpath = std::string(argv[1]);
	}
	else {
		tcout << T_TEXT("argument error! press enter to exit.") << endl;
		getchar();
		return 0;
	}
	
	//创建节点
	CenterNode nd;
	if (!nd.Init(cfgpath)) {
		tcout << T_TEXT("init error! press enter to exit.") << endl;
		getchar();
		return 0;
	}
	
	tcout << T_TEXT("start working! press enter to stop and exit.") << endl;
	getchar();
	return 0;
}
