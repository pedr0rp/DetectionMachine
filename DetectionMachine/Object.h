#ifndef OBJECT_H
#define OBJECT_H

#include "opencv2/imgproc/imgproc.hpp",
#include "opencv2/highgui/highgui.hpp"

enum Shape { CIRCLE = 0, RECTANGLE = 1, TRIANGLE = 2 };
static const char *shapeString[] = { "CIRCLE", "RECTANGLE", "TRIANGLE" };

class Object {

private:
	cv::Point lastPosition;
	cv::Point position;
	std::string name;
	cv::Mat tracking;
	Shape shape;
	std::string color;

public:

	virtual ~Object() {};

	void setName(std::string value);
	std::string getName();

	void setPosition(cv::Point value);
	cv::Point getPosition();
	cv::Mat getTracking();
	
	void setShape(Shape value);
	Shape getShape();
	void setColor(std::string value);
	std::string getColor();

};

#endif 