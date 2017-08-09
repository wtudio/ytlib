#include "t_process.h"

namespace ytlib
{

	bool test_ProcessBase() {

		return true;
	}
	bool test_AlgProcess() {

		return true;
	}

	class testmsg {
	public:
		testmsg(int n) :index(n) {

		}
		int index;
	};

	class testQueueProcess : public QueueProcess<std::shared_ptr<testmsg> > {
	public:
		testQueueProcess(size_t thCount_ = 1, size_t queueSize_ = 1000):QueueProcess(thCount_, queueSize_){}
		virtual ~testQueueProcess(){ stop(); }
	protected:
		void ProcFun(const std::shared_ptr<testmsg>& ptr_) {
			printf("%d\n", ptr_->index);
		}
	};

	bool test_QueueProcess() {
	
		testQueueProcess testprocess;
		testprocess.init();
		testprocess.start();
		for (int ii = 0; ii < 1000; ii++) {
			testprocess.Add(std::shared_ptr<testmsg>(new testmsg(ii)));
		}
		//Sleep(500);
		for (int ii = 1000; ii < 2000; ii++) {
			testprocess.Add(std::shared_ptr<testmsg>(new testmsg(ii)));
		}

		//Sleep(500);

		testprocess.stop();
		printf("*************************************\n");
		//Sleep(500);
		testprocess.start();
		testprocess.init();
		testprocess.start();

		for (int ii = 3000; ii < 4000; ii++) {
			testprocess.Add(std::shared_ptr<testmsg>(new testmsg(ii)));
		}
		testprocess.stop();
		return true;
	}
}