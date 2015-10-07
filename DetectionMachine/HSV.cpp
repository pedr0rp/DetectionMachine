#include "HSV.h"

std::string HSV::getWindowName()
{
	return getName() + " control";
}

void HSV::showControl() {

	cv::namedWindow(getWindowName(), cv::WINDOW_NORMAL);

	cv::createTrackbar("LOW_H", getWindowName(), &lowH, 179); 
	cv::createTrackbar("HIGH_H", getWindowName(), &highH, 179);

	cv::createTrackbar("LOW_S", getWindowName(), &lowS, 255); 
	cv::createTrackbar("HIGH_S", getWindowName(), &highS, 255);

	cv::createTrackbar("LOW_V", getWindowName(), &lowV, 255);
	cv::createTrackbar("HIGH_V", getWindowName(), &highV, 255);
}

cv::Scalar HSV::getLow() { 
	return cv::Scalar(lowH, lowS, lowV);
}

cv::Scalar HSV::getHigh(){
	return cv::Scalar(highH, highS, highV);
}

void HSV::setLow(int lowH, int lowS, int lowV) {
	HSV::lowH = lowH;
	HSV::lowS = lowS;
	HSV::lowV = lowV;
}

void HSV::setHigh(int highH, int highS, int highV) {
	HSV::highH = highH;
	HSV::highS = highS;
	HSV::highV = highV;
}

void HSV::setName(std::string value) {
	name = value;
}

std::string HSV::getName() {
	return name;
}

std::vector<std::string> HSV::splitString(const std::string &s, char delim) {
	std::stringstream ss(s);
	std::string item;
	std::vector<std::string> tokens;
	while (getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}

void HSV::save() {
	std::ofstream o(DIRECTORY + name + FILE_EXTENSION);
	std::string content = "";
	content += std::to_string((int)getLow()[0]) + ";";
	content += std::to_string((int)getLow()[1]) + ";";
	content += std::to_string((int)getLow()[2]) + ";";
	content += std::to_string((int)getHigh()[0]) + ";";
	content += std::to_string((int)getHigh()[1]) + ";";
	content += std::to_string((int)getHigh()[2]) + ";";

	o << content << std::endl;
}

bool HSV::load() {
	std::ifstream infile;

	char data[100];
	infile.open(DIRECTORY + name + FILE_EXTENSION);
	if (infile.is_open()) {
		infile >> data;
		infile.close();

		std::string content(data);
		std::vector<std::string> v = splitString(content, ';');
		setLow(std::stoi(v[0]), std::stoi(v[1]), std::stoi(v[2]));
		setHigh(std::stoi(v[3]), std::stoi(v[4]), std::stoi(v[5]));

		return true;
	} else {
		return false;
	}
}


