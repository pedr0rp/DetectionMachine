#ifndef FPS_H
#define FPS_H

#include <iomanip>
#include <sstream>
#include <string>
#include <time.h>

class FPS {

private:
	time_t _start, _end;
	int counter = 0;
	double sec;
	double fps;

public:
	void start();
	std::string end();
};

#endif 
