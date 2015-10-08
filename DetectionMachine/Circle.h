#ifndef CIRCLE_H
#define CIRCLE_H

#include "Object.h"

class Circle : public Object {

private:
	int radius;

public:
	void setRadius(int value);
	int getRadius();
};

#endif 