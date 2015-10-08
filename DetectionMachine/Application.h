#ifndef APPLICATION_H
#define APPLICATION_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp",
#include <fstream>
#include <future>
#include <vector>
#include "Serial.h"
#include "FPS.h"
#include "HSV.h""
#include "Object.h";
#include "Circle.h";


#define MAX_COLOR	10
#define MAX_OBJECT  20
#define CAM			1	

class Application {

private:
	bool TRACKING = false;
	bool SHOW_PROCESSED = true;
	bool INCLUDE_TEXT = true;
	bool INCLUDE_AIM = false;
	bool INCLUDE_DISTANCE = true;

	FPS framerate;
	Serial* SP;

	int source;
	char imageFile[20];

	HSV colors[MAX_COLOR];
	int colorCount;

	std::vector<Object> objects;	
	int objectCount = 0;

public:
	bool init();
	int start();
	void calibrate();
	cv::Mat readImage();
	static cv::Mat threshold(cv::Mat src, HSV color);
	static cv::Mat preprocessing(cv::Mat src, HSV color);
	static std::vector<Object> findCircles(cv::Mat src, HSV color);
	static std::vector<Object> findPoly(cv::Mat src, HSV color);
	static void drawObject(cv::Mat &src, Object object);
};


#endif 