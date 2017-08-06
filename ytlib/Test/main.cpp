#include <ytlib/Common/Util.h>
#include <ytlib/SupportTools/Serialize.h>
#include <ytlib/SupportTools/UUID.h>
#include <ytlib/NetTools/TcpNetAdapter.h>
#include <ytlib/LogService/LoggerServer.h>
#include <ytlib/LogService/Logger.h>

#include "mathtest.h"
#include "filetest.h"
#include "processtest.h"
#include "nettest.h"

#include <boost/date_time/posix_time/posix_time.hpp>  

using namespace std;
using namespace ytlib;

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace keywords = boost::log::keywords;
namespace expr = boost::log::expressions;
//
//BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)

int32_t main(int32_t argc, char** argv) {
	YT_DEBUG_PRINTF("-------------------start-------------------\n");
	//tcout输出中文需要设置
	//建议：最好不要在程序中使用中文！！！
	//std::locale::global(std::locale(""));
	//wcout.imbue(locale(""));


	//boost::posix_time::ptime ticTime_global, tocTime_global;
	//ticTime_global = boost::posix_time::microsec_clock::universal_time();

	//tocTime_global = boost::posix_time::microsec_clock::universal_time(); 
	//std::cout << (tocTime_global - ticTime_global).ticks() << "us" << std::endl;

	

	//src::logger_mt& lg = my_logger::get();
	//BOOST_LOG(lg) << "Greetings from the global logger!";

	boost::shared_ptr< logging::core > core = logging::core::get();

	typedef boost::log::sinks::synchronous_sink< NetBackend > sink_t;
	
	boost::shared_ptr< sink_t > sink(new sink_t());

	sink->set_formatter(
		expr::stream<<"["
		<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")
		<< "]: <" << logging::trivial::severity
		<< "> " << expr::smessage
	);
	core->add_sink(sink);
	core->remove_sink(sink);
	logging::add_common_attributes();

	BOOST_LOG_TRIVIAL(debug) << "Hello, it's a simple notification";


	test_TcpNetAdapter();
	
	LoggerServer l(55555);
	l.start();


	test_Complex();
	test_Matrix();
	test_Matrix_c();

	test_KeyValueFile();
	test_SerializeFile();
	test_XMLFile();
	test_PrjBase();

	test_QueueProcess();
	
	printf("******************end*******************\n");
	getchar();
	
}
