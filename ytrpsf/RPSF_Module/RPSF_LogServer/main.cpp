#include <ytlib/LogService/LoggerServer.h>
#include <ytlib/FileManager/KeyValueFile.h>


using namespace ytlib;

int32_t main(int32_t argc, char** argv) {
	tcout << T_TEXT("---------Log Server---------") << std::endl;
	//获取配置文件路径
	tstring cfgpath;
	if (argc == 1) {
		cfgpath = (tGetCurrentPath() / T_TEXT("LogServer.lcfg")).string<tstring>();
	}
	else if (argc == 2) {
		cfgpath = T_STRING_TO_TSTRING(std::string(argv[1]));
	}
	else {
		tcout << T_TEXT("argument error! press enter to exit.") << std::endl;
		getchar();
		return 0;
	}

	//读取配置文件
	KeyValueFile LogServerCfg;
	try {
		LogServerCfg.OpenFile(cfgpath);
	}
	catch (const Exception& e) {
		std::cout << "open cfg file failed:" << e.what() << std::endl;
		getchar();
		return 0;
	}

	//开启日志服务器
	std::shared_ptr<std::map<std::string, std::string> > pcfgmap = LogServerCfg.GetFileObjPtr();
	std::map<std::string, std::string>::const_iterator itr = pcfgmap->find("port");
	if (itr == pcfgmap->end()) {
		tcout << T_TEXT("invalid cfg file") << std::endl;
		getchar();
		return 0;
	}
	uint16_t port = atoi(itr->second.c_str());
	itr= pcfgmap->find("logPath");
	tstring logPath;
	if (itr != pcfgmap->end()) logPath = T_STRING_TO_TSTRING(itr->second);

	tcout << T_TEXT("listen to port: ") << port << std::endl;
	tcout << T_TEXT("log path: ") << logPath << std::endl;

	LoggerServer ls(port, logPath);
	if (!ls.start()) {
		tcout << T_TEXT("log server start failed. port: ")<< port<< T_TEXT(" may be already in used")<< std::endl;
		getchar();
		return 0;
	}
	tcout << T_TEXT("log server start successful! press enter to stop and exit") << std::endl;
	//关闭时自动退出
	getchar();
	return 0;
}
