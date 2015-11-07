#include "opencv2/highgui/highgui.hpp"
#include <iostream>

using namespace cv;
using namespace std;

int m1n(int argc, char* argv[])
{
	VideoCapture cap(1); 

	if (!cap.isOpened()) 
	{
		cout << "Cannot open the video cam" << endl;
		return -1;
	}

	double dWidth = cap.get(CV_CAP_PROP_FRAME_WIDTH); 
	double dHeight = cap.get(CV_CAP_PROP_FRAME_HEIGHT); 

	cout << "Frame size : " << dWidth << " x " << dHeight << endl;

	namedWindow("MyVideo", CV_WINDOW_AUTOSIZE); //create a window called "MyVideo"

	while (1)
	{
		Mat frame;
		double t = (double)cv::getTickCount();
		bool bSuccess = cap.read(frame);

		if (!bSuccess)
		{
			cout << "Cannot read a frame from video stream" << endl;
			break;
		}

		imshow("MyVideo", frame); 
		t = ((double)cv::getTickCount() - t) / cv::getTickFrequency();// elapsed time in ms
		std::cout << "I am working at " << 1000.0 / t << " FPS" << std::endl;

		if (waitKey(30) == 27) 
		{
			cout << "esc key is pressed by user" << endl;
			break;
		}
	}
	return 0;

}