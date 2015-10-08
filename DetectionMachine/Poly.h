#ifndef POLY_H
#define POLY_H

#include "Object.h"

class Poly : public Object {

private:
	std::vector<cv::Point> v;

public:
	void setV(std::vector<cv::Point> value);
	std::vector<cv::Point> getV();
};

#endif 