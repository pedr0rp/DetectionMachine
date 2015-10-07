#ifndef HSV_H
#define HSV_H

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp",
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#define FILE_EXTENSION ".hsv"
#define DIRECTORY "colors\\"

class HSV {

private:
	std::string name;

	int lowH = 0;
	int highH = 179;
	int lowS = 0;
	int highS = 255;
	int lowV = 0;
	int highV = 255;

	std::string getWindowName();

public:
	
	void showControl();

	cv::Scalar getLow();
	cv::Scalar getHigh();
	void setLow(int lowH, int lowS, int lowV);
	void setHigh(int highH, int highS, int highV);

	void setName(std::string value);
	std::string getName();

	void save();
	bool load();

	std::vector<std::string> splitString(const std::string &s, char delim);

};

#endif 