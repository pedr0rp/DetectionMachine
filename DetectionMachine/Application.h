#ifndef APPLICATION_H
#define APPLICATION_H

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <fstream>
#include <future>
#include <vector>
#include "Serial.h"
#include "FPS.h"
#include "HSV.h""
#include "Object.h";
#include "Circle.h";
#include "Poly.h";


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

	cv::VideoCapture cap = cv::VideoCapture(CAM);
	FPS framerate;
	Serial* SP;

	int source;
	char imageFile[20];

	HSV colors[MAX_COLOR];
	int colorCount;

	std::vector<Object*> objects;		

	int index;

public:
	bool init();
	int start();
	void calibrate();
	cv::Mat readImage();
	static cv::Mat threshold(cv::Mat src, HSV color);
	static cv::Mat preprocessing(cv::Mat src, HSV color);
	static std::vector<Circle*> findCircles(cv::Mat src, HSV color);
	static std::vector<Poly*> findPoly(cv::Mat src, HSV color);
	static void drawObject(cv::Mat &src, Object* object);
	static void drawCircle(cv::Mat &src, Circle* circle);
	static void drawPoly(cv::Mat &src, Poly* poly);
	static double angle(cv::Point p1, cv::Point p2, cv::Point p0);
	static float distance(cv::Point p0, cv::Point p1);
	static void mouseCapture(int event, int x, int y, int flags, void* userdata);
};


#endif 