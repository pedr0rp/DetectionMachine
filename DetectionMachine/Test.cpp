#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int counta(Mat src)  {
	int i = 0;
	int j = 0;

	int count = 0;
	while (i < src.rows) {
		while (j < src.cols) {
			if (src.at<uchar>(i, j) == 255) {
				count++;
			}
			j++;
		}
		i++;
	}
	return count;
}

int maazin(int argc, char* argv[])
{
	cv::Mat img = cv::imread("a.png", 0);



	printf("testeeeeeeeeeeee\n");
	int i = 0;
	int j = 0;
	bool flag = false;

	printf("%d", counta(img));

	return 0;

}