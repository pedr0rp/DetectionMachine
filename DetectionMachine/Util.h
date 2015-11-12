#ifndef UTIL_H
#define UTIL_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <fstream>
#include <future>
#include <vector>
#include <time.h>
#include <iomanip>
#include "Arduino.h"
#include "Serial.h"
#include "FPS.h"
#include "HSV.h""
#include "Object.h";
#include "Circle.h";
#include "Poly.h";


#define MAX_COLOR	10
#define MAX_OBJECT  20

class Util {

private:
	static bool TRACKING;
	static bool SHOW_PROCESSED;
	static bool INCLUDE_TEXT;
	static bool INCLUDE_AIM;
	static bool INCLUDE_DISTANCE;

	static int CAM;
	static cv::VideoCapture cap;

	const static int fileSize = 20;
	static char imageFile[fileSize];

	static const float resizeRatio;


public:
	static cv::Mat readImage();
	static cv::Mat threshold(cv::Mat src, HSV color);
	static cv::Mat preprocessing(cv::Mat src, HSV color);
	static std::vector<Circle*> findCircles(cv::Mat src, HSV color);
	static std::vector<Poly*> findPoly(cv::Mat src, HSV color);
	static void drawObject(cv::Mat &src, Object* object);
	static void drawCircle(cv::Mat &src, Circle* circle);
	static void drawPoly(cv::Mat &src, Poly* poly);
	static double angle(cv::Point p1, cv::Point p2, cv::Point p0);
	static float distance(cv::Point p0, cv::Point p1);
	static std::vector<cv::Point> removeNear(std::vector<cv::Point> src);
	static cv::Mat resize(cv::Mat src);
	static void setCam(int value);
	static bool camIsOpened();
	static void setImage(char value[fileSize]);

};


#endif 