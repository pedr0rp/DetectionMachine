#include "Object.h";

//Object::~Object() {}

//Object::Object(cv::Mat src) {
//	position.x = -1;
//	position.y = -1;
//	lastPosition = position;
//	tracking = cv::Mat::zeros(src.size(), CV_8UC3);
//}

void Object::setName(std::string value) { name = value; }
std::string Object::getName() { return name; }

void Object::setPosition(cv::Point value) {

	if (lastPosition.x != -1 && lastPosition.y != -1) {
		line(tracking, lastPosition, value, cv::Scalar(0, 0, 255), 2, 8);
	}

	lastPosition = position;
	position = value;
}

cv::Point Object::getPosition() { return position; }

cv::Mat Object::getTracking() { return tracking; }

void Object::setShape(Shape value) { shape = value; }
Shape Object::getShape() { return shape; }

void Object::setColor(std::string value) { color = value; }
std::string Object::getColor() { return color; }

