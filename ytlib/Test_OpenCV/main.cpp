#include <ytlib/Common/Util.h>
#include <boost/date_time/posix_time/posix_time.hpp>  
#include<opencv2\opencv.hpp>

using namespace std;
//using namespace ytlib;
using namespace cv;

int32_t main(int32_t argc, char** argv) {
	YT_DEBUG_PRINTF("-------------------start-------------------\n");

	Mat img1 = imread("map.jpg");
	imshow("map", img1);
	

	printf("******************end*******************\n");
	getchar();
}
