#ifndef FPS_H
#define FPS_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <time.h>

#define FILE_EXTENSION ".fps"
#define DIRECTORY "framerate\\"

class FPS {

private:
	time_t _start, _end;
	int counter = 0;
	double sec;
	double fps;
	bool fileCreated = false;
	std::string fileName;

public:
	void start();
	std::string end();
	void save(std::string value);
};

#endif 
